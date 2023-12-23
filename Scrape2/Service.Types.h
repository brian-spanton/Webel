// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Uri.h"
#include "Basic.SuffixArray.h"
#include "Json.Types.h"

namespace Service
{
    typedef Basic::SuffixArray<std::shared_ptr<Json::Object> > Index;
}