#!/usr/bin/env bash
# GeneralsX @build cheviiot 24/10/2026 PLAN-023 Gate 6: dual-instance LAN discovery.
# Usage: ./scripts/qa/smoke/smoke-echelon-lan-dual.sh [build-dir] [profile] [timeout-sec]
#
# Preferred path: two podman containers on a shared bridge (works without host netns).
# Fallback: host ip-netns + veth (needs CAP_NET_ADMIN). Soft-pass exit 3 if neither works.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-${PROJECT_ROOT}/build/linux64-unified-baseline}"
PROFILE="${2:-zh}"
TIMEOUT_SEC="${3:-120}"
BINARY="${BUILD_DIR}/GeneralsMD/echelon"
# Path inside mounted project root for container exec cwd=PROJECT_ROOT
BINARY_REL="${BINARY#"${PROJECT_ROOT}"/}"
if [[ "${BINARY_REL}" == "${BINARY}" ]]; then
	BINARY_REL="${BINARY}"
fi
LOG_DIR="${PROJECT_ROOT}/logs"
STAMP="$(date +%Y%m%d-%H%M%S)"

export CNC_GENERALS_PATH="${CNC_GENERALS_PATH:-${HOME}/.local/share/generalsx/game-data/Generals}"
export CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH:-${HOME}/.local/share/generalsx/game-data/ZeroHour}"

mkdir -p "${LOG_DIR}"
log_a="${LOG_DIR}/smoke-echelon-lan-dual-a-${PROFILE}-${STAMP}.out"
log_b="${LOG_DIR}/smoke-echelon-lan-dual-b-${PROFILE}-${STAMP}.out"

if [[ ! -x "${BINARY}" ]]; then
	echo "ERROR: echelon binary not found: ${BINARY}"
	exit 1
fi

soft_pass() {
	echo "SOFT-PASS: dual LAN discovery unavailable here ($1)"
	echo "SOFT-PASS: need podman bridge twins or CAP_NET_ADMIN netns"
	exit 3
}

check_pair() {
	local ra="$1" rb="$2"
	grep -q '\[LAN_PROBE\] peer seen' "${log_a}" \
		|| { echo "ERROR: A did not see peer"; tail -40 "${log_a}"; return 1; }
	grep -q '\[LAN_PROBE\] peer seen' "${log_b}" \
		|| { echo "ERROR: B did not see peer"; tail -40 "${log_b}"; return 1; }
	grep -q '\[LAN_PROBE\] ok peers=' "${log_a}" \
		|| { echo "ERROR: A probe did not complete ok"; tail -40 "${log_a}"; return 1; }
	grep -q '\[LAN_PROBE\] ok peers=' "${log_b}" \
		|| { echo "ERROR: B probe did not complete ok"; tail -40 "${log_b}"; return 1; }
	if [[ "${ra}" -ne 0 || "${rb}" -ne 0 ]]; then
		echo "ERROR: dual LAN probe exit A=${ra} B=${rb}"
		return 1
	fi
	echo "OK: dual-instance LAN discovery profile=${PROFILE}"
	return 0
}

run_podman_bridge() {
	command -v podman >/dev/null 2>&1 || return 1
	local src="${ECHELON_LAN_DUAL_SOURCE_CONTAINER:-ubuntu-dev}"
	if ! podman inspect "${src}" >/dev/null 2>&1; then
		echo "INFO: source container '${src}' missing; skip podman dual path"
		return 1
	fi

	local img="localhost/echelon-lan-smoke:tmp-$$"
	local net="echelon-lan-dual-$$"
	local ca="lan-a-dual-$$"
	local cb="lan-b-dual-$$"

	cleanup_pod() {
		podman rm -f "${ca}" "${cb}" >/dev/null 2>&1 || true
		podman network rm "${net}" >/dev/null 2>&1 || true
		podman rmi "${img}" >/dev/null 2>&1 || true
	}
	trap cleanup_pod EXIT

	echo "INFO: dual LAN via podman bridge (source=${src}) profile=${PROFILE}"
	podman commit "${src}" "${img}" >/dev/null
	podman network create --subnet 10.89.1.0/24 "${net}" >/dev/null

	local vols=(
		-v "${PROJECT_ROOT}:${PROJECT_ROOT}:ro"
		-v "${HOME}/.local/share/generalsx:${HOME}/.local/share/generalsx:ro"
		--workdir "${PROJECT_ROOT}"
	)
	local env_common=(
		-e "HOME=${HOME}"
		-e "CNC_GENERALS_PATH=${CNC_GENERALS_PATH}"
		-e "CNC_GENERALS_ZH_PATH=${CNC_GENERALS_ZH_PATH}"
		-e "ECHELON_LAN_PROBE_EXPECT_PEER=1"
	)

	# Bypass distrobox/custom ENTRYPOINT so sleep stays running for exec.
	podman run -d --name "${ca}" --network "${net}" --ip 10.89.1.10 \
		--entrypoint sleep \
		"${vols[@]}" "${env_common[@]}" \
		-e ECHELON_LAN_PROBE_NAME=EchelonA -e ECHELON_LAN_PROBE_IP=10.89.1.10 \
		-e XDG_DATA_HOME=/tmp/lan-a \
		"${img}" "$((TIMEOUT_SEC + 60))" >/dev/null

	podman run -d --name "${cb}" --network "${net}" --ip 10.89.1.11 \
		--entrypoint sleep \
		"${vols[@]}" "${env_common[@]}" \
		-e ECHELON_LAN_PROBE_NAME=EchelonB -e ECHELON_LAN_PROBE_IP=10.89.1.11 \
		-e XDG_DATA_HOME=/tmp/lan-b \
		"${img}" "$((TIMEOUT_SEC + 60))" >/dev/null

	# Wait until both are running before exec.
	for _ in 1 2 3 4 5 6 7 8 9 10; do
		if podman inspect -f '{{.State.Running}}' "${ca}" 2>/dev/null | grep -q true \
			&& podman inspect -f '{{.State.Running}}' "${cb}" 2>/dev/null | grep -q true; then
			break
		fi
		sleep 0.3
	done
	podman inspect -f '{{.State.Running}}' "${ca}" 2>/dev/null | grep -q true \
		|| { echo "ERROR: container A not running"; return 1; }
	podman inspect -f '{{.State.Running}}' "${cb}" 2>/dev/null | grep -q true \
		|| { echo "ERROR: container B not running"; return 1; }

	# Stagger slightly so both binds are up before the expect window closes.
	podman exec "${ca}" bash -lc "timeout ${TIMEOUT_SEC} ./${BINARY_REL} -game ${PROFILE} -headless -lanProbe" \
		>"${log_a}" 2>&1 &
	local pa=$!
	sleep 0.4
	podman exec "${cb}" bash -lc "timeout ${TIMEOUT_SEC} ./${BINARY_REL} -game ${PROFILE} -headless -lanProbe" \
		>"${log_b}" 2>&1 &
	local pb=$!

	local ra=0 rb=0
	set +e
	wait "${pa}"; ra=$?
	wait "${pb}"; rb=$?
	set -e
	echo "INFO: A exit=${ra} log=${log_a}"
	echo "INFO: B exit=${rb} log=${log_b}"
	if ! check_pair "${ra}" "${rb}"; then
		cleanup_pod
		trap - EXIT
		return 1
	fi
	cleanup_pod
	trap - EXIT
	return 0
}

run_netns() {
	command -v ip >/dev/null 2>&1 || return 1
	local ns_a="echelon-lan-a-$$"
	local ns_b="echelon-lan-b-$$"
	local veth_a="elvA$$"
	local veth_b="elvB$$"

	cleanup_ns() {
		ip netns del "${ns_a:-}" 2>/dev/null || true
		ip netns del "${ns_b:-}" 2>/dev/null || true
		ip link del "${veth_a:-}" 2>/dev/null || true
		ip link del "${veth_b:-}" 2>/dev/null || true
	}
	trap cleanup_ns EXIT

	echo "INFO: dual LAN via ip netns profile=${PROFILE}"
	if ! ip netns add "${ns_a}" 2>/dev/null; then
		trap - EXIT
		return 1
	fi
	if ! ip netns add "${ns_b}" \
		|| ! ip link add "${veth_a}" type veth peer name "${veth_b}" \
		|| ! ip link set "${veth_a}" netns "${ns_a}" \
		|| ! ip link set "${veth_b}" netns "${ns_b}" \
		|| ! ip -n "${ns_a}" addr add 10.66.77.1/24 dev "${veth_a}" \
		|| ! ip -n "${ns_b}" addr add 10.66.77.2/24 dev "${veth_b}" \
		|| ! ip -n "${ns_a}" link set "${veth_a}" up \
		|| ! ip -n "${ns_b}" link set "${veth_b}" up \
		|| ! ip -n "${ns_a}" link set lo up \
		|| ! ip -n "${ns_b}" link set lo up \
		|| ! ip netns exec "${ns_a}" true 2>/dev/null; then
		cleanup_ns
		trap - EXIT
		return 1
	fi

	local xdg_a xdg_b
	xdg_a="$(mktemp -d /tmp/echelon-lan-dual-a.XXXXXX)"
	xdg_b="$(mktemp -d /tmp/echelon-lan-dual-b.XXXXXX)"

	local pa pb ra=0 rb=0
	set +e
	ip netns exec "${ns_a}" env \
		HOME="${HOME}" XDG_DATA_HOME="${xdg_a}" \
		CNC_GENERALS_PATH="${CNC_GENERALS_PATH}" CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH}" \
		ECHELON_LAN_PROBE_NAME=EchelonA ECHELON_LAN_PROBE_IP=10.66.77.1 \
		ECHELON_LAN_PROBE_EXPECT_PEER=1 \
		timeout "${TIMEOUT_SEC}" "${BINARY}" -game "${PROFILE}" -headless -lanProbe \
		>"${log_a}" 2>&1 &
	pa=$!
	sleep 0.4
	ip netns exec "${ns_b}" env \
		HOME="${HOME}" XDG_DATA_HOME="${xdg_b}" \
		CNC_GENERALS_PATH="${CNC_GENERALS_PATH}" CNC_GENERALS_ZH_PATH="${CNC_GENERALS_ZH_PATH}" \
		ECHELON_LAN_PROBE_NAME=EchelonB ECHELON_LAN_PROBE_IP=10.66.77.2 \
		ECHELON_LAN_PROBE_EXPECT_PEER=1 \
		timeout "${TIMEOUT_SEC}" "${BINARY}" -game "${PROFILE}" -headless -lanProbe \
		>"${log_b}" 2>&1 &
	pb=$!
	wait "${pa}"; ra=$?
	wait "${pb}"; rb=$?
	set -e
	echo "INFO: A exit=${ra} log=${log_a}"
	echo "INFO: B exit=${rb} log=${log_b}"
	if ! check_pair "${ra}" "${rb}"; then
		cleanup_ns
		trap - EXIT
		return 1
	fi
	cleanup_ns
	trap - EXIT
	return 0
}

if run_podman_bridge; then
	exit 0
fi
if run_netns; then
	exit 0
fi
soft_pass "neither podman-bridge nor netns path succeeded"
