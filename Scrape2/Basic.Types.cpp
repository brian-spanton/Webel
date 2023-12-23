// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Types.h"
#include "Basic.Globals.h"

namespace Basic
{
    FatalError::FatalError(const char* context)
    {
        Basic::globals->HandleError(context, 0);
    }

    FatalError::FatalError(const char* context, uint32 error)
    {
        Basic::globals->HandleError(context, error);
    }
}
