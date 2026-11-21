# GeneralsX WebGPU (WASM) runtime container image
# Built from the release asset Web-WebGPU-GeneralsXZH.zip extracted into BUNDLE_DIR.
FROM nginx:1.27-alpine

ARG BUNDLE_DIR=dist-wasm-bundle

RUN rm -rf /usr/share/nginx/html/*

COPY web/nginx.conf /etc/nginx/conf.d/default.conf
COPY ${BUNDLE_DIR}/ /usr/share/nginx/html/

RUN rm -f /usr/share/nginx/html/serve-webgpu.py

EXPOSE 80

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD wget -qO- http://127.0.0.1/healthz >/dev/null || exit 1
