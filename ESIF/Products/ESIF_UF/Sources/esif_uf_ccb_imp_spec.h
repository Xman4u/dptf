/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#include "esif.h"
#include "esif_uf_ipc.h"

#ifdef ESIF_FEAT_OPT_ACTION_SYSFS
void SysfsRegisterParticipants();
void SysfsRegisterCustomParticipant(char *targetHID, char *targetACPIName, UInt32 pType);
eEsifError EsifActSysfsInit(void);
void EsifActSysfsExit(void);

static ESIF_INLINE void esif_ccb_participants_initialize()
{
	SysfsRegisterParticipants();
	#ifdef ESIF_ATTR_OS_ANDROID
	if (!EsifUpPm_DoesAvailableParticipantExistByHID("INT3406")) {
		SysfsRegisterCustomParticipant("INT3406","TDSP", (UInt32) ESIF_DOMAIN_TYPE_INVALID);
	}
	if (!EsifUpPm_DoesAvailableParticipantExistByHID("INT3408")) {
		SysfsRegisterCustomParticipant("INT3408","WWAN", (UInt32) ESIF_DOMAIN_TYPE_WWAN);
	}
	#endif
}

static ESIF_INLINE void esif_ccb_imp_spec_actions_init()
{
	EsifActSysfsInit();
}

static ESIF_INLINE void esif_ccb_imp_spec_actions_exit()
{
	EsifActSysfsExit();
}
#else
enum esif_rc sync_lf_participants();
static ESIF_INLINE void esif_ccb_participants_initialize()
{
	ipc_connect();
	sync_lf_participants();
}

#define esif_ccb_imp_spec_actions_init()
#define esif_ccb_imp_spec_actions_exit()
#endif
