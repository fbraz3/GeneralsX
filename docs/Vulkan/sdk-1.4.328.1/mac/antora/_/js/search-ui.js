(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? factory(exports) :
  typeof define === 'function' && define.amd ? define(['exports'], factory) :
  (global = typeof globalThis !== 'undefined' ? globalThis : global || self, factory(global.antoraSearch = {}));
})(this, (function (exports) { 'use strict';

  /**
   * Splitting the text by the given positions.
   * The text within the positions getting the type "mark", all other text gets the type "text".
   * @param {string} text
   * @param {Object[]} positions
   * @param {number} positions.start
   * @param {number} positions.length
   * @param {number} snippetLength Maximum text length for text in the result.
   * @returns {[{text: string, type: string}]}
   */
  function buildHighlightedText (text, positions, snippetLength) {
    const textLength = text.length;
    const validPositions = positions
      .filter((position) => position.length > 0 && position.start + position.length <= textLength);

    if (validPositions.length === 0) {
      return [
        {
          type: 'text',
          text: text.slice(0, snippetLength >= textLength ? textLength : snippetLength) + (snippetLength < textLength ? '...' : ''),
        },
      ]
    }

    const orderedPositions = validPositions.sort((p1, p2) => p1.start - p2.start);
    const range = {
      start: 0,
      end: textLength,
    };
    const firstPosition = orderedPositions[0];
    if (snippetLength && text.length > snippetLength) {
      const firstPositionStart = firstPosition.start;
      const firstPositionLength = firstPosition.length;
      const firstPositionEnd = firstPositionStart + firstPositionLength;

      range.start = firstPositionStart - snippetLength < 0 ? 0 : firstPositionStart - snippetLength;
      range.end = firstPositionEnd + snippetLength > textLength ? textLength : firstPositionEnd + snippetLength;
    }
    const nodes = [];
    if (firstPosition.start > 0) {
      nodes.push({
        type: 'text',
        text: (range.start > 0 ? '...' : '') + text.slice(range.start, firstPosition.start),
      });
    }
    let lastEndPosition = 0;
    const positionsWithinRange = orderedPositions
      .filter((position) => position.start >= range.start && position.start + position.length <= range.end);

    for (const position of positionsWithinRange) {
      const start = position.start;
      const length = position.length;
      const end = start + length;
      if (lastEndPosition > 0) {
        // create text Node from the last end position to the start of the current position
        nodes.push({
          type: 'text',
          text: text.slice(lastEndPosition, start),
        });
      }
      nodes.push({
        type: 'mark',
        text: text.slice(start, end),
      });
      lastEndPosition = end;
    }
    if (lastEndPosition < range.end) {
      nodes.push({
        type: 'text',
        text: text.slice(lastEndPosition, range.end) + (range.end < textLength ? '...' : ''),
      });
    }

    return nodes
  }

  /**
   * Taken and adapted from: https://github.com/olivernn/lunr.js/blob/aa5a878f62a6bba1e8e5b95714899e17e8150b38/lib/tokenizer.js#L24-L67
   * @param lunr
   * @param text
   * @param term
   * @return {{start: number, length: number}}
   */
  function findTermPosition (lunr, term, text) {
    const str = text.toLowerCase();
    // const len = str.length

    // experiment with avoiding regex
    const index = str.indexOf(term);
    const len = str.substr(index).match(/^[^.,\s]*/)[0].length;

    if (index === -1) {
      // Not found
      return {
        start: 0,
        length: 0,
      }
    } else {
      return {
        start: index,
        length: len,
      }
    }
  }

  class TrieNode {
    constructor () {
      this.children = new Map();
      this.isEndOfWord = false;
      this.data = []; // Store associated data (e.g., document IDs, URLs)
    }
  }

  class LevenshteinTrieUser {
    constructor () {
      this.root = new TrieNode();
    }

    insert (word) {
      let node = this.root;
      for (const char of word) {
        if (!node.children.has(char)) {
          node.children.set(char, new TrieNode());
        }
        node = node.children.get(char);
      }
      node.isEndOfWord = true;
    }

    searchWithLevenshtein (word, maxDistance) {
      const results = [];
      this._searchRecursive(this.root, '', word, 0, maxDistance, results);
      return results
    }

    _searchRecursive (node, currentWord, targetWord, currentIndex, maxDistance, results) {
      if (currentIndex > targetWord.length && node.isEndOfWord) {
        results.push(currentWord);
        return
      }
      if (maxDistance < 0) {
        return
      }

      if (node.isEndOfWord && this.levenshteinDistance(currentWord, targetWord) <= maxDistance) {
        results.push(currentWord);
      }

      for (const [char, childNode] of node.children) {
        let newDistance = maxDistance;
        if (currentIndex < targetWord.length) {
          if (char === targetWord[currentIndex]) {
            this._searchRecursive(childNode, currentWord + char, targetWord, currentIndex + 1, newDistance, results);
          } else {
            newDistance = maxDistance - 1; //substitution
            this._searchRecursive(childNode, currentWord + char, targetWord, currentIndex + 1, newDistance, results);
            this._searchRecursive(node, currentWord, targetWord, currentIndex + 1, newDistance, results); //insertion
            this._searchRecursive(
              childNode,
              currentWord + char,
              targetWord,
              currentIndex,
              newDistance,
              results
            ); // deletion
          }
        } else {
          this._searchRecursive(childNode, currentWord + char, targetWord, currentIndex, newDistance - 1, results);
        }
      }
    }

    levenshteinDistance (a, b) {
      if (a.length === 0) return b.length
      if (b.length === 0) return a.length

      const matrix = [];

      // increment along the first column of each row
      let i;
      for (i = 0; i <= b.length; i++) {
        matrix[i] = [i];
      }

      // increment each column in the first row
      let j;
      for (j = 0; j <= a.length; j++) {
        matrix[0][j] = j;
      }

      // Fill in the rest of the matrix
      for (i = 1; i <= b.length; i++) {
        for (j = 1; j <= a.length; j++) {
          if (b.charAt(i - 1) === a.charAt(j - 1)) {
            matrix[i][j] = matrix[i - 1][j - 1];
          } else {
            matrix[i][j] = Math.min(
              matrix[i - 1][j - 1] + 1, // substitution
              Math.min(matrix[i][j - 1] + 1, // insertion
                matrix[i - 1][j] + 1) // deletion
            );
          }
        }
      }

      return matrix[b.length][a.length]
    }

    // Save the Trie to a JSON string
    save () {
      return JSON.stringify(this.root, (key, value) => {
        if (value instanceof Map) {
          return Array.from(value.entries()) // Convert Map to array of entries
        }
        return value
      })
    }

    // Load the Trie from a JSON string
    load (jsonString) {
      this.root = jsonString;
      // this.root = JSON.parse(jsonString, (key, value) => {
      //   if (Array.isArray(value)) {
      //     return new Map(value) // Convert array of entries back to Map
      //   }
      //   return value
      // })
    }

    insertWithData (word, data) {
      let node = this.root;
      for (const char of word) {
        if (!node.children.has(char)) {
          node.children.set(char, new TrieNode());
        }
        node = node.children.get(char);
      }
      node.isEndOfWord = true;
      node.data.push(data); // Store the associated data
    }

    searchWithLevenshteinWithData (word, maxDistance) {
      const results = [];
      this._searchRecursiveWithData(this.root, '', word, 0, maxDistance, results);
      return results
    }

    _searchRecursiveWithData (node, currentWord, targetWord, currentIndex, maxDistance, results) {
      if (currentIndex > targetWord.length && node.isEndOfWord) {
        results.push({ word: currentWord, data: node.data });
        return
      }
      if (maxDistance < 0) {
        return
      }

      if (node.isEndOfWord && this.levenshteinDistance(currentWord, targetWord) <= maxDistance) {
        results.push({ word: currentWord, data: node.data });
      }

      for (const [char, childNode] of node.children) {
        let newDistance = maxDistance;
        if (currentIndex < targetWord.length) {
          if (char === targetWord[currentIndex]) {
            this._searchRecursiveWithData(
              childNode,
              currentWord + char,
              targetWord,
              currentIndex + 1,
              newDistance,
              results
            );
          } else {
            newDistance = maxDistance - 1; //substitution
            this._searchRecursiveWithData(
              childNode,
              currentWord + char,
              targetWord,
              currentIndex + 1,
              newDistance,
              results
            );
            this._searchRecursiveWithData(
              node,
              currentWord,
              targetWord,
              currentIndex + 1,
              newDistance,
              results
            ); //insertion
            this._searchRecursiveWithData(
              childNode,
              currentWord + char,
              targetWord,
              currentIndex,
              newDistance,
              results
            ); // deletion
          }
        } else {
          this._searchRecursiveWithData(childNode, currentWord + char, targetWord, currentIndex, newDistance - 1, results);
        }
      }
    }
  }

  /* global CustomEvent, globalThis */

  const config = document.getElementById('search-ui-script').dataset;
  const snippetLength = parseInt(config.snippetLength || 100, 10);
  const siteRootPath = config.siteRootPath || '';
  appendStylesheet(config.stylesheet);
  const searchInput = document.getElementById('search-input');
  const searchResultContainer = document.createElement('div');
  searchResultContainer.classList.add('search-result-dropdown-menu');
  searchInput.parentNode.appendChild(searchResultContainer);
  const facetFilterInput = document.querySelector('#search-field input[type=checkbox][data-facet-filter]');

  function appendStylesheet (href) {
    if (!href) return
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = href;
    document.head.appendChild(link);
  }

  function highlightPageTitle (title, terms) {
    const positions = getTermPosition(title, terms);
    return buildHighlightedText(title, positions, snippetLength)
  }

  function highlightSectionTitle (sectionTitle, terms) {
    if (sectionTitle) {
      const text = sectionTitle.text;
      const positions = getTermPosition(text, terms);
      return buildHighlightedText(text, positions, snippetLength)
    }
    return []
  }

  function highlightKeyword (doc, terms) {
    const keyword = doc.keyword;
    if (keyword) {
      const positions = getTermPosition(keyword, terms);
      return buildHighlightedText(keyword, positions, snippetLength)
    }
    return []
  }

  function highlightText (doc, terms) {
    const text = doc.text;
    const positions = getTermPosition(text, terms);
    return buildHighlightedText(text, positions, snippetLength)
  }

  function getTermPosition (text, terms) {
    const positions = terms
      .map((term) => findTermPosition(globalThis.lunr, term, text))
      .filter((position) => position.length > 0)
      .sort((p1, p2) => p1.start - p2.start);

    if (positions.length === 0) {
      return []
    }
    return positions
  }

  function highlightHit (searchMetadata, sectionTitle, doc) {
    const terms = {};
    for (const term in searchMetadata) {
      const fields = searchMetadata[term];
      for (const field in fields) {
        terms[field] = [...(terms[field] || []), term];
      }
    }
    return {
      pageTitleNodes: highlightPageTitle(doc.title, terms.title || []),
      sectionTitleNodes: highlightSectionTitle(sectionTitle, terms.title || []),
      pageContentNodes: highlightText(doc, terms.text || []),
      pageKeywordNodes: highlightKeyword(doc, terms.keyword || []),
    }
  }

  function createSearchResult (result, store, searchResultDataset) {
    let currentComponent;
    result.forEach(function (item) {
      const ids = item.ref.split('-');
      const docId = ids[0];
      const doc = store.documents[docId];
      let sectionTitle;
      if (ids.length > 1) {
        const titleId = ids[1];
        sectionTitle = doc.titles.filter(function (item) {
          return String(item.id) === titleId
        })[0];
      }
      const metadata = item.matchData.metadata;
      const highlightingResult = highlightHit(metadata, sectionTitle, doc);
      const componentVersion = store.componentVersions[`${doc.component}/${doc.version}`];
      if (componentVersion !== undefined && currentComponent !== componentVersion) {
        const searchResultComponentHeader = document.createElement('div');
        searchResultComponentHeader.classList.add('search-result-component-header');
        const { title, displayVersion } = componentVersion;
        const componentVersionText = `${title}${doc.version && displayVersion ? ` ${displayVersion}` : ''}`;
        searchResultComponentHeader.appendChild(document.createTextNode(componentVersionText));
        searchResultDataset.appendChild(searchResultComponentHeader);
        currentComponent = componentVersion;
      }
      searchResultDataset.appendChild(createSearchResultItem(doc, sectionTitle, item, highlightingResult));
    });
  }

  function createSearchResultItem (doc, sectionTitle, item, highlightingResult) {
    const documentTitle = document.createElement('div');
    documentTitle.classList.add('search-result-document-title');
    highlightingResult.pageTitleNodes.forEach(function (node) {
      let element;
      if (node.type === 'text') {
        element = document.createTextNode(node.text);
      } else {
        element = document.createElement('span');
        element.classList.add('search-result-highlight');
        element.innerText = node.text;
      }
      documentTitle.appendChild(element);
    });
    const documentHit = document.createElement('div');
    documentHit.classList.add('search-result-document-hit');
    const documentHitLink = document.createElement('a');
    documentHitLink.href = siteRootPath + doc.url + (sectionTitle ? '#' + sectionTitle.hash : '');
    documentHit.appendChild(documentHitLink);
    if (highlightingResult.sectionTitleNodes.length > 0) {
      const documentSectionTitle = document.createElement('div');
      documentSectionTitle.classList.add('search-result-section-title');
      documentHitLink.appendChild(documentSectionTitle);
      highlightingResult.sectionTitleNodes.forEach((node) => createHighlightedText(node, documentSectionTitle));
    }
    highlightingResult.pageContentNodes.forEach((node) => createHighlightedText(node, documentHitLink));

    // only show keyword when we got a hit on them
    if (doc.keyword && highlightingResult.pageKeywordNodes.length > 1) {
      const documentKeywords = document.createElement('div');
      documentKeywords.classList.add('search-result-keywords');
      const documentKeywordsFieldLabel = document.createElement('span');
      documentKeywordsFieldLabel.classList.add('search-result-keywords-field-label');
      documentKeywordsFieldLabel.innerText = 'keywords: ';
      const documentKeywordsList = document.createElement('span');
      documentKeywordsList.classList.add('search-result-keywords-list');
      highlightingResult.pageKeywordNodes.forEach((node) => createHighlightedText(node, documentKeywordsList));
      documentKeywords.appendChild(documentKeywordsFieldLabel);
      documentKeywords.appendChild(documentKeywordsList);
      documentHitLink.appendChild(documentKeywords);
    }
    const searchResultItem = document.createElement('div');
    searchResultItem.classList.add('search-result-item');
    searchResultItem.appendChild(documentTitle);
    searchResultItem.appendChild(documentHit);
    searchResultItem.addEventListener('mousedown', function (e) {
      e.preventDefault();
    });
    return searchResultItem
  }

  /**
   * Creates an element from a highlightingResultNode and add it to the targetNode.
   * @param {Object} highlightingResultNode
   * @param {String} highlightingResultNode.type - type of the node
   * @param {String} highlightingResultNode.text
   * @param {Node} targetNode
   */
  function createHighlightedText (highlightingResultNode, targetNode) {
    let element;
    if (highlightingResultNode.type === 'text') {
      element = document.createTextNode(highlightingResultNode.text);
    } else {
      element = document.createElement('span');
      element.classList.add('search-result-highlight');
      element.innerText = highlightingResultNode.text;
    }
    targetNode.appendChild(element);
  }

  function createNoResult (text) {
    const searchResultItem = document.createElement('div');
    searchResultItem.classList.add('search-result-item');
    const documentHit = document.createElement('div');
    documentHit.classList.add('search-result-document-hit');
    const message = document.createElement('strong');
    message.innerText = 'No results found for query "' + text + '"';
    documentHit.appendChild(message);
    searchResultItem.appendChild(documentHit);
    return searchResultItem
  }

  function clearSearchResults (reset) {
    if (reset === true) searchInput.value = '';
    searchResultContainer.innerHTML = '';
  }

  function filter (result, documents) {
    const facetFilter = facetFilterInput && facetFilterInput.checked && facetFilterInput.dataset.facetFilter;
    if (facetFilter) {
      const [field, value] = facetFilter.split(':');
      return result.filter((item) => {
        const ids = item.ref.split('-');
        const docId = ids[0];
        const doc = documents[docId];
        return field in doc && doc[field] === value
      })
    }
    return result
  }

  function search (index, documents, queryString) {
    // execute an exact match search
    let query;
    let result = filter(
      index.query(function (lunrQuery) {
        const parser = new globalThis.lunr.QueryParser(queryString, lunrQuery);
        parser.parse();
        query = lunrQuery;
      }),
      documents
    );
    if (result.length > 0) {
      return result
    }
    // no result, use a begins with search
    result = filter(
      index.query(function (lunrQuery) {
        lunrQuery.clauses = query.clauses.map((clause) => {
          if (clause.presence !== globalThis.lunr.Query.presence.PROHIBITED) {
            clause.term = clause.term + '*';
            clause.wildcard = globalThis.lunr.Query.wildcard.TRAILING;
            clause.usePipeline = false;
          }
          return clause
        });
      }),
      documents
    );
    if (result.length > 0) {
      return result
    }
    // no result, use a contains search
    result = filter(
      index.query(function (lunrQuery) {
        lunrQuery.clauses = query.clauses.map((clause) => {
          if (clause.presence !== globalThis.lunr.Query.presence.PROHIBITED) {
            clause.term = '*' + clause.term + '*';
            clause.wildcard = globalThis.lunr.Query.wildcard.LEADING | globalThis.lunr.Query.wildcard.TRAILING;
            clause.usePipeline = false;
          }
          return clause
        });
      }),
      documents
    );
    return result
  }

  function searchIndex (index, trie, store, text) {
    clearSearchResults(false);
    if (text.trim() === '') {
      return
    }
    const maxLevenshteinDistance = 3;
    const trieResults = trie
      .searchWithLevenshteinWithData(text.toLowerCase(), maxLevenshteinDistance);
    let result;
    const recheck = /\s/.test(text);
    if (!trieResults) {
      result = search(index, store.documents, text);
      if (recheck) {
        result = search(index, store.documents, text.replace(/\s/g, '_'));
      }
    } else {
      // Extract unique document IDs from Trie results
      const trieDocIds = new Set();
      trieResults.forEach((r) => r.data.forEach((d) => trieDocIds.add(d)));

      let lunrResults = [];
      if (trieDocIds.size > 0) {
        // Filter documents for Lunr search
        const filteredDocuments = [];
        trieDocIds.forEach((id) => {
          filteredDocuments.push(store.documents[id]);
        });
        if (filteredDocuments.length > 0) {
          // Rebuild a temporary index only with the filtered documents
          const tempLunrIndex = globalThis.lunr(function () {
            this.ref('id');
            this.field('title', { boost: 10 });
            this.field('name');
            this.field('text');
            this.field('component');
            this.field('keyword', { boost: 5 });
            filteredDocuments.forEach((doc) => this.add(doc));
          });
          lunrResults = search(tempLunrIndex, filteredDocuments, text);
          if (recheck) {
            result = search(index, store.documents, text.replace(/\s/g, '_'));
          }
        } else {
          lunrResults = search(index, store.documents, text);
          if (recheck) {
            result = search(index, store.documents, text.replace(/\s/g, '_'));
          }
        }
      } else {
        lunrResults = search(index, store.documents, text);
        if (recheck) {
          result = search(index, store.documents, text.replace(/\s/g, '_'));
        }
      }
      result = lunrResults;
    }
    const searchResultDataset = document.createElement('div');
    searchResultDataset.classList.add('search-result-dataset');
    searchResultContainer.appendChild(searchResultDataset);
    if (result.length > 0) {
      createSearchResult(result, store, searchResultDataset);
    } else {
      searchResultDataset.appendChild(createNoResult(text));
    }
  }

  function confineEvent (e) {
    e.stopPropagation();
  }

  function debounce (func, wait, immediate) {
    let timeout;
    return function () {
      const context = this;
      const args = arguments;
      const later = function () {
        timeout = null;
        if (!immediate) func.apply(context, args);
      };
      const callNow = immediate && !timeout;
      clearTimeout(timeout);
      timeout = setTimeout(later, wait);
      if (callNow) func.apply(context, args);
    }
  }

  function enableSearchInput (enabled) {
    if (facetFilterInput) {
      facetFilterInput.disabled = !enabled;
    }
    searchInput.disabled = !enabled;
    searchInput.title = enabled ? '' : 'Loading index...';
  }

  function isClosed () {
    return searchResultContainer.childElementCount === 0
  }

  function executeSearch (index) {
    const debug = 'URLSearchParams' in globalThis && new URLSearchParams(globalThis.location.search).has('lunr-debug');
    const query = searchInput.value;
    try {
      if (!query) return clearSearchResults()
      searchIndex(index.index, index.trie, index.store, query);
    } catch (err) {
      if (err instanceof globalThis.lunr.QueryParseError) {
        if (debug) {
          console.debug('Invalid search query: ' + query + ' (' + err.message + ')');
        }
      } else {
        console.error('Something went wrong while searching', err);
      }
    }
  }

  function toggleFilter (e, index) {
    searchInput.focus();
    if (!isClosed()) {
      executeSearch(index);
    }
  }

  function base64ToBytesArr (str) {
    const abc = [...'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/']; // base64 alphabet
    const result = [];

    for (let i = 0; i < str.length / 4; i++) {
      const chunk = [...str.slice(4 * i, 4 * i + 4)];
      const bin = chunk.map((x) => abc.indexOf(x).toString(2).padStart(6, 0)).join('');
      const bytes = bin.match(/.{1,8}/g).map((x) => +('0b' + x));
      result.push(...bytes.slice(0, 3 - (str[4 * i + 2] === '=') - (str[4 * i + 3] === '=')));
    }
    return result
  }

  function initSearch (lunr, data, trieData) {
    const start = performance.now();
    data = base64ToBytesArr(data);
    data = window.pako.inflate(data, { to: 'string' });
    const lunrdata = JSON.parse(data);
    trieData = base64ToBytesArr(trieData);
    const trieDataJSON = window.pako.inflate(trieData, { to: 'string' });
    const index = { index: lunr.Index.load(lunrdata.index), store: lunrdata.store, trie: new LevenshteinTrieUser() };
    index.trie.load(JSON.parse(trieDataJSON));
    enableSearchInput(true);
    searchInput.dispatchEvent(
      new CustomEvent('loadedindex', {
        detail: {
          took: performance.now() - start,
        },
      })
    );
    searchInput.addEventListener(
      'keydown',
      debounce(function (e) {
        if (e.key === 'Escape' || e.key === 'Esc') return clearSearchResults(true)
        executeSearch(index);
      }, 100)
    );
    searchInput.addEventListener('click', confineEvent);
    searchResultContainer.addEventListener('click', confineEvent);
    if (facetFilterInput) {
      facetFilterInput.parentElement.addEventListener('click', confineEvent);
      facetFilterInput.addEventListener('change', (e) => toggleFilter(e, index));
    }
    document.documentElement.addEventListener('click', clearSearchResults);
  }

  // disable the search input until the index is loaded
  enableSearchInput(false);

  exports.initSearch = initSearch;

  Object.defineProperty(exports, '__esModule', { value: true });

}));
