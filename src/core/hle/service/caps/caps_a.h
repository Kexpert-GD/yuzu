// Copyright 2020 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Kernel {
class HLERequestContext;
}

namespace Service::Capture {

class CAPS_A final : public ServiceFramework<CAPS_A> {
public:
    explicit CAPS_A();
    ~CAPS_A() override;
};

} // namespace Service::Capture