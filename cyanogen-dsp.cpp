/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "DSP-entry"

#include <cutils/log.h>
#include <string.h>
#include <media/AudioEffect.h>
#include <hardware/audio_effect.h>
#include <audio_effects/effect_bassboost.h>
#include <audio_effects/effect_equalizer.h>
#include <audio_effects/effect_virtualizer.h>
#include <audio_effects/effect_stereowide.h>

#include "Effect.h"
#include "EffectBassBoost.h"
#include "EffectCompression.h"
#include "EffectEqualizer.h"
#include "EffectVirtualizer.h"
#include "EffectStereoWide.h"

static effect_descriptor_t compression_descriptor = {
	{ 0x09e8ede0, 0xddde, 0x11db, 0xb4f6, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } }, // SL_IID_VOLUME
	{ 0xf27317f4, 0xc984, 0x4de6, 0x9a90, { 0x54, 0x57, 0x59, 0x49, 0x5b, 0xf2 } }, // own UUID
	EFFECT_CONTROL_API_VERSION,
	EFFECT_FLAG_INSERT_FIRST | EFFECT_FLAG_VOLUME_CTRL,
	10, /* 1 MIPS. FIXME: should be measured. */
	1,
	"CyanogenMod's Dynamic Range Compression",
	"Antti S. Lankila"
};

static effect_descriptor_t virtualizer_descriptor = {
	*SL_IID_VIRTUALIZER,
	{ 0x7c6cc5f8, 0x6f34, 0x4449, 0xa282, { 0xbe, 0xd8, 0x4f, 0x1a, 0x5b, 0x5a } }, // own UUID
	EFFECT_CONTROL_API_VERSION,
	EFFECT_FLAG_INSERT_LAST,
	10, /* 1 MIPS. FIXME: should be measured. */
	1,
	"CyanogenMod's Headset Virtualization",
	"Antti S. Lankila"
};

static effect_descriptor_t stereowide_descriptor = {
	*SL_IID_STEREOWIDE,
        /* 37cc2c00-dddd-11db-8577-0002a5d5c51c */
	{ 0x37cc2c00, 0xdddd, 0x11db, 0x8577, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1c } }, // own UUID
	EFFECT_CONTROL_API_VERSION,
	EFFECT_FLAG_INSERT_LAST,
	10, /* 1 MIPS. FIXME: should be measured. */
	1,
	"OmniROM's Stereo Widener",
	"Guillaume Lesniak"
};

static effect_descriptor_t equalizer_descriptor = {
	*SL_IID_EQUALIZER,
        { 0x58bc9000, 0x0d7f, 0x462e, 0x90d2, { 0x03, 0x5e, 0xdd, 0xd8, 0xb4, 0x34 } }, // own UUID
	EFFECT_CONTROL_API_VERSION,
	0,
	10, /* 1 MIPS. FIXME: should be measured. */
	1,
	"CyanogenMod's Equalizer",
	"Antti S. Lankila"
};

static effect_descriptor_t bassboost_descriptor = {
	*SL_IID_BASSBOOST,
	{ 0x42b5cbf5, 0x4dd8, 0x4e79, 0xa5fb, { 0xcc, 0xeb, 0x2c, 0xb5, 0x4e, 0x13 } }, // own UUID
	EFFECT_CONTROL_API_VERSION,
	0,
	10, /* 1 MIPS. FIXME: should be measured. */
	1,
	"CyanogenMod's Bass Boost",
	"Antti S. Lankila"
};

/* Library mandatory methods. */
extern "C" {

struct effect_module_s {
	const struct effect_interface_s *itfe;
	Effect *effect;
	effect_descriptor_t *descriptor;
};

static int32_t generic_process(effect_handle_t self, audio_buffer_t *in, audio_buffer_t *out) {
	struct effect_module_s *e = (struct effect_module_s *) self;
	return e->effect->process(in, out);
}

static int32_t generic_command(effect_handle_t self, uint32_t cmdCode, uint32_t cmdSize, void *pCmdData, uint32_t *replySize, void *pReplyData) {
	struct effect_module_s *e = (struct effect_module_s *) self;
	return e->effect->command(cmdCode, cmdSize, pCmdData, replySize, pReplyData);
}

static int32_t generic_getDescriptor(effect_handle_t self, effect_descriptor_t *pDescriptor) {
	struct effect_module_s *e = (struct effect_module_s *) self;
	memcpy(pDescriptor, e->descriptor, sizeof(effect_descriptor_t));
	return 0;
}

static const struct effect_interface_s generic_interface = {
	generic_process,
	generic_command,
	generic_getDescriptor,
	NULL
};

int32_t EffectCreate(const effect_uuid_t *uuid, int32_t sessionId, int32_t ioId, effect_handle_t *pEffect) {
	if (memcmp(uuid, &compression_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
		struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
		e->itfe = &generic_interface;
		e->effect = new EffectCompression();
		e->descriptor = &compression_descriptor;
		*pEffect = (effect_handle_t) e;
		return 0;
	}
	if (memcmp(uuid, &equalizer_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
		struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
		e->itfe = &generic_interface;
		e->effect = new EffectEqualizer();
		e->descriptor = &equalizer_descriptor;
		*pEffect = (effect_handle_t) e;
		return 0;
	}
	if (memcmp(uuid, &virtualizer_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
		struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
		e->itfe = &generic_interface;
		e->effect = new EffectVirtualizer();
		e->descriptor = &virtualizer_descriptor;
		*pEffect = (effect_handle_t) e;
		return 0;
	}
	if (memcmp(uuid, &bassboost_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
		struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
		e->itfe = &generic_interface;
		e->effect = new EffectBassBoost();
		e->descriptor = &bassboost_descriptor;
		*pEffect = (effect_handle_t) e;
		return 0;
	}
        if (memcmp(uuid, &stereowide_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
		struct effect_module_s *e = (struct effect_module_s *) calloc(1, sizeof(struct effect_module_s));
		e->itfe = &generic_interface;
		e->effect = new EffectStereoWide();
		e->descriptor = &stereowide_descriptor;
		*pEffect = (effect_handle_t) e;
		return 0;
	}

	return -EINVAL;
}

int32_t EffectRelease(effect_handle_t ei) {
	struct effect_module_s *e = (struct effect_module_s *) ei;
	delete e->effect;
	free(e);
	return 0;
}

int32_t EffectGetDescriptor(const effect_uuid_t *uuid, effect_descriptor_t *pDescriptor) {
	if (memcmp(uuid, &compression_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
	    memcpy(pDescriptor, &compression_descriptor, sizeof(effect_descriptor_t));
	    return 0;
	}
	if (memcmp(uuid, &equalizer_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
	    memcpy(pDescriptor, &equalizer_descriptor, sizeof(effect_descriptor_t));
	    return 0;
	}
	if (memcmp(uuid, &virtualizer_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
	    memcpy(pDescriptor, &virtualizer_descriptor, sizeof(effect_descriptor_t));
	    return 0;

	}
	if (memcmp(uuid, &bassboost_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
	    memcpy(pDescriptor, &bassboost_descriptor, sizeof(effect_descriptor_t));
	    return 0;
	}
        if (memcmp(uuid, &stereowide_descriptor.uuid, sizeof(effect_uuid_t)) == 0) {
	    memcpy(pDescriptor, &stereowide_descriptor, sizeof(effect_descriptor_t));
	    return 0;
	}

	return -EINVAL;
}

audio_effect_library_t AUDIO_EFFECT_LIBRARY_INFO_SYM = {
    .tag = AUDIO_EFFECT_LIBRARY_TAG,
    .version = EFFECT_LIBRARY_API_VERSION,
    .name = "CyanogenMod's Effect Library",
    .implementor = "Antti S. Lankila",
    .create_effect = EffectCreate,
    .release_effect = EffectRelease,
    .get_descriptor = EffectGetDescriptor,
};

}
