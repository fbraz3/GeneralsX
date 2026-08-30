#include "NativeWebRTCProtocol.h"

using GeneralsX::NativeWebRTC::CurrentBuildCompatibilityProfile;

static_assert(CurrentBuildCompatibilityProfile().engine == EXPECTED_ENGINE_COMPATIBILITY);
static_assert(CurrentBuildCompatibilityProfile().protocol == GENERALSX_COMPAT_PROTOCOL);
static_assert(CurrentBuildCompatibilityProfile().determinism == EXPECTED_DETERMINISM_COMPATIBILITY);

int main()
{
	return 0;
}
