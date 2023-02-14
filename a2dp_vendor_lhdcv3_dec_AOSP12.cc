/******************************************************************************
 *
 *  Copyright 2002-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Utility functions to help build and parse SBC Codec Information Element
 *  and Media Payload.
 *
 ******************************************************************************/

#define LOG_TAG "a2dp_vendor_lhdcv3_dec"

#include "bt_target.h"

#include "a2dp_vendor_lhdcv3_dec.h"

#include <string.h>

#include <base/logging.h>
#include "a2dp_vendor.h"
#include "a2dp_vendor_lhdcv3_decoder.h"
#include "bt_utils.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"


// data type for the LHDC Codec Information Element */
// NOTE: bits_per_sample is needed only for LHDC encoder initialization.
typedef struct {
  uint32_t vendorId;
  uint16_t codecId;    /* Codec ID for LHDC */
  uint8_t sampleRate;  /* Sampling Frequency */
  btav_a2dp_codec_bits_per_sample_t bits_per_sample;
  uint8_t channelSplitMode;
  uint8_t version;
  uint8_t maxTargetBitrate;
  bool isLLSupported;
  //uint8_t supportedBitrate;
  bool hasFeatureJAS;
  bool hasFeatureAR;
  bool hasFeatureLLAC;
  bool hasFeatureMETA;
  bool hasFeatureMinBitrate;
  bool hasFeatureLARC;
  bool hasFeatureLHDCV4;
} tA2DP_LHDCV3_SINK_CIE;

/* LHDC Sink codec capabilities */
static const tA2DP_LHDCV3_SINK_CIE a2dp_lhdcv3_sink_caps = {
    A2DP_LHDC_VENDOR_ID,  // vendorId
    A2DP_LHDCV3_CODEC_ID,   // codecId
    // sampleRate
    //(A2DP_LHDC_SAMPLING_FREQ_48000),
    (A2DP_LHDC_SAMPLING_FREQ_44100 | A2DP_LHDC_SAMPLING_FREQ_48000 | A2DP_LHDC_SAMPLING_FREQ_88200 | A2DP_LHDC_SAMPLING_FREQ_96000),
    // bits_per_sample
    (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),
    //Channel Separation
    //A2DP_LHDC_CH_SPLIT_NONE | A2DP_LHDC_CH_SPLIT_TWS,
	A2DP_LHDC_CH_SPLIT_NONE,
    //Version number
    A2DP_LHDC_VER3,
    //Target bit Rate
    A2DP_LHDC_MAX_BIT_RATE_900K,
    //LL supported ?
    true,

    /*******************************
     *  LHDC features/capabilities:
     *  hasFeatureJAS
     *  hasFeatureAR
     *  hasFeatureLLAC
     *  hasFeatureMETA
     *  hasFeatureMinBitrate
     *  hasFeatureLARC
     *  hasFeatureLHDCV4
     *******************************/
    //bool hasFeatureJAS;
    true,

    //bool hasFeatureAR;
    true,

    //bool hasFeatureLLAC;
    true,

    //bool hasFeatureMETA;
    true,

    //bool hasFeatureMinBitrate;
    true,

    //bool hasFeatureLARC;
    false,

    //bool hasFeatureLHDCV4;
    true,
};

/* Default LHDC codec configuration */
static const tA2DP_LHDCV3_SINK_CIE a2dp_lhdcv3_sink_default_config = {
    A2DP_LHDC_VENDOR_ID,                // vendorId
    A2DP_LHDCV3_CODEC_ID,                 // codecId
    A2DP_LHDC_SAMPLING_FREQ_96000,      // sampleRate
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24,  // bits_per_sample
    A2DP_LHDC_CH_SPLIT_NONE,
    A2DP_LHDC_VER3,
    A2DP_LHDC_MAX_BIT_RATE_900K,
    false,

    //bool hasFeatureJAS;
    false,

    //bool hasFeatureAR;
    false,

    //bool hasFeatureLLAC;
    true,

    //bool hasFeatureMETA;
    false,

    //bool hasFeatureMinBitrate;
    true,

    //bool hasFeatureLARC;
    false,

    //bool hasFeatureLHDCV4;
    true,
};


static const tA2DP_LHDCV3_SINK_CIE a2dp_lhdcv3_sink_v4_config = {
    A2DP_LHDC_VENDOR_ID,                // vendorId
    A2DP_LHDCV3_CODEC_ID,                 // codecId
    (A2DP_LHDC_SAMPLING_FREQ_44100 | A2DP_LHDC_SAMPLING_FREQ_48000),
    (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),  // bits_per_sample
    A2DP_LHDC_CH_SPLIT_NONE,
    A2DP_LHDC_VER3,
    A2DP_LHDC_MAX_BIT_RATE_900K,
    //LL supported ?
    false,

    //bool hasFeatureJAS;
    false,

    //bool hasFeatureAR;
    false,

    //bool hasFeatureLLAC;
    true,

    //bool hasFeatureMETA;
    false,

    //bool hasFeatureMinBitrate;
    true,

    //bool hasFeatureLARC;
    false,

    //bool hasFeatureLHDCV4;
    false,
};

static const tA2DP_LHDCV3_SINK_CIE a2dp_lhdcv3_sink_v3_config = {
    A2DP_LHDC_VENDOR_ID,                // vendorId
    A2DP_LHDCV3_CODEC_ID,                 // codecId
    (A2DP_LHDC_SAMPLING_FREQ_44100 | A2DP_LHDC_SAMPLING_FREQ_48000 | A2DP_LHDC_SAMPLING_FREQ_96000),
    (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),  // bits_per_sample
    A2DP_LHDC_CH_SPLIT_NONE,
    A2DP_LHDC_VER3,
    A2DP_LHDC_MAX_BIT_RATE_900K,
    //LL supported ?
    false,

    //bool hasFeatureJAS;
    false,

    //bool hasFeatureAR;
    false,

    //bool hasFeatureLLAC;
    false,

    //bool hasFeatureMETA;
    false,

    //bool hasFeatureMinBitrate;
    false,

    //bool hasFeatureLARC;
    false,

    //bool hasFeatureLHDCV4;
    false,
};


static const tA2DP_DECODER_INTERFACE a2dp_decoder_interface_lhdcv3 = {
    a2dp_vendor_lhdcv3_decoder_init,
    a2dp_vendor_lhdcv3_decoder_cleanup,
    a2dp_vendor_lhdcv3_decoder_decode_packet,
    a2dp_vendor_lhdcv3_decoder_start,
    a2dp_vendor_lhdcv3_decoder_suspend,
    a2dp_vendor_lhdcv3_decoder_configure,
};

static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityLhdcV3Sink(
    const tA2DP_LHDCV3_SINK_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_capability);


// Builds the LHDC Media Codec Capabilities byte sequence beginning from the
// LOSC octet. |media_type| is the media type |AVDT_MEDIA_TYPE_*|.
// |p_ie| is a pointer to the LHDC Codec Information Element information.
// The result is stored in |p_result|. Returns A2DP_SUCCESS on success,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_BuildInfoLhdcV3Sink(uint8_t media_type,
                                       const tA2DP_LHDCV3_SINK_CIE* p_ie,
                                       uint8_t* p_result) {

  const uint8_t* tmpInfo = p_result;
  if (p_ie == NULL || p_result == NULL) {
    return A2DP_INVALID_PARAMS;
  }

  *p_result++ = A2DP_LHDCV3_CODEC_LEN;    //0
  *p_result++ = (media_type << 4);      //1
  *p_result++ = A2DP_MEDIA_CT_NON_A2DP; //2

  // Vendor ID and Codec ID
  *p_result++ = (uint8_t)(p_ie->vendorId & 0x000000FF); //3
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x0000FF00) >> 8);  //4
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x00FF0000) >> 16); //5
  *p_result++ = (uint8_t)((p_ie->vendorId & 0xFF000000) >> 24); //6
  *p_result++ = (uint8_t)(p_ie->codecId & 0x00FF);  //7
  *p_result++ = (uint8_t)((p_ie->codecId & 0xFF00) >> 8);   //8

  // Sampling Frequency & Bits per sample
  uint8_t para = 0;

  // sample rate bit0 ~ bit2
  para = (uint8_t)(p_ie->sampleRate & A2DP_LHDC_SAMPLING_FREQ_MASK);

  if (p_ie->bits_per_sample == (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16)) {
      para = para | (A2DP_LHDC_BIT_FMT_24 | A2DP_LHDC_BIT_FMT_16);
  }else if(p_ie->bits_per_sample == BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24){
      para = para | A2DP_LHDC_BIT_FMT_24;
  }else if(p_ie->bits_per_sample == BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16){
      para = para | A2DP_LHDC_BIT_FMT_16;
  }

  if (p_ie->hasFeatureJAS)
  {
    para |= A2DP_LHDC_FEATURE_JAS;
  }

  if (p_ie->hasFeatureAR)
  {
    para |= A2DP_LHDC_FEATURE_AR;
  }

  // Save octet 9
  *p_result++ = para;   //9

  para = p_ie->version;

  para |= p_ie->maxTargetBitrate;

  para |= p_ie->isLLSupported ? A2DP_LHDC_LL_SUPPORTED : A2DP_LHDC_LL_NONE;

  if (p_ie->hasFeatureLLAC)
  {
    para |= A2DP_LHDC_FEATURE_LLAC;
  }

  // Save octet 10
  *p_result++ = para;   //a

  //Save octet 11
  para = p_ie->channelSplitMode;

  if (p_ie->hasFeatureMETA)
  {
    para |= A2DP_LHDC_FEATURE_META;
  }

  if (p_ie->hasFeatureMinBitrate)
  {
    para |= A2DP_LHDC_FEATURE_MIN_BR;
  }

  if (p_ie->hasFeatureLARC)
  {
    para |= A2DP_LHDC_FEATURE_LARC;
  }

  if (p_ie->hasFeatureLHDCV4)
  {
    para |= A2DP_LHDC_FEATURE_LHDCV4;
  }

  *p_result++ = para;   //b

  //Save octet 12
  //para = p_ie->supportedBitrate;
  //*p_result++ = para;   //c

  LOG_DEBUG("%s: Info build result = [0]:0x%x, [1]:0x%x, [2]:0x%x, [3]:0x%x, "
                     "[4]:0x%x, [5]:0x%x, [6]:0x%x, [7]:0x%x, [8]:0x%x, [9]:0x%x, [10]:0x%x, [11]:0x%x",
     __func__, tmpInfo[0], tmpInfo[1], tmpInfo[2], tmpInfo[3],
                    tmpInfo[4], tmpInfo[5], tmpInfo[6], tmpInfo[7], tmpInfo[8], tmpInfo[9], tmpInfo[10], tmpInfo[11]);
  return A2DP_SUCCESS;
}

// Parses the LHDC Media Codec Capabilities byte sequence beginning from the
// LOSC octet. The result is stored in |p_ie|. The byte sequence to parse is
// |p_codec_info|. If |is_capability| is true, the byte sequence is
// codec capabilities, otherwise is codec configuration.
// Returns A2DP_SUCCESS on success, otherwise the corresponding A2DP error
// status code.
static tA2DP_STATUS A2DP_ParseInfoLhdcV3Sink(tA2DP_LHDCV3_SINK_CIE* p_ie,
                                       const uint8_t* p_codec_info,
                                       bool is_capability) {
  uint8_t losc;
  uint8_t media_type;
  tA2DP_CODEC_TYPE codec_type;
  const uint8_t* tmpInfo = p_codec_info;
  const uint8_t* p_codec_Info_save = p_codec_info;

  //LOG_DEBUG("%s: p_ie = %p, p_codec_info = %p", __func__, p_ie, p_codec_info);
  if (p_ie == NULL || p_codec_info == NULL) return A2DP_INVALID_PARAMS;

  // Check the codec capability length
  losc = *p_codec_info++;

  if (losc != A2DP_LHDCV3_CODEC_LEN) return A2DP_WRONG_CODEC;

  media_type = (*p_codec_info++) >> 4;
  codec_type = *p_codec_info++;
    //LOG_DEBUG("%s: media_type = %d, codec_type = %d", __func__, media_type, codec_type);
  /* Check the Media Type and Media Codec Type */
  if (media_type != AVDT_MEDIA_TYPE_AUDIO ||
      codec_type != A2DP_MEDIA_CT_NON_A2DP) {
    return A2DP_WRONG_CODEC;
  }

  // Check the Vendor ID and Codec ID */
  p_ie->vendorId = (*p_codec_info & 0x000000FF) |
                   (*(p_codec_info + 1) << 8 & 0x0000FF00) |
                   (*(p_codec_info + 2) << 16 & 0x00FF0000) |
                   (*(p_codec_info + 3) << 24 & 0xFF000000);
  p_codec_info += 4;
  p_ie->codecId =
      (*p_codec_info & 0x00FF) | (*(p_codec_info + 1) << 8 & 0xFF00);
  p_codec_info += 2;
  LOG_DEBUG("%s:Vendor(0x%08x), Codec(0x%04x)", __func__, p_ie->vendorId, p_ie->codecId);
  if (p_ie->vendorId != A2DP_LHDC_VENDOR_ID ||
      p_ie->codecId != A2DP_LHDCV3_CODEC_ID) {
    return A2DP_WRONG_CODEC;
  }

  p_ie->sampleRate = *p_codec_info & A2DP_LHDC_SAMPLING_FREQ_MASK;
  if ((*p_codec_info & A2DP_LHDC_BIT_FMT_MASK) == 0) {
    return A2DP_WRONG_CODEC;
  }

  p_ie->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
  if (*p_codec_info & A2DP_LHDC_BIT_FMT_24)
    p_ie->bits_per_sample |= BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
  if (*p_codec_info & A2DP_LHDC_BIT_FMT_16) {
    p_ie->bits_per_sample |= BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
  }

  p_ie->hasFeatureJAS = ((*p_codec_info & A2DP_LHDC_FEATURE_JAS) != 0) ? true : false;
  p_ie->hasFeatureAR = ((*p_codec_info & A2DP_LHDC_FEATURE_AR) != 0) ? true : false;
  p_codec_info += 1;

  p_ie->version = (*p_codec_info) & A2DP_LHDC_VERSION_MASK;
  //p_ie->version = 1;

  p_ie->maxTargetBitrate = (*p_codec_info) & A2DP_LHDC_MAX_BIT_RATE_MASK;
  //p_ie->maxTargetBitrate = A2DP_LHDC_MAX_BIT_RATE_900K;

  p_ie->isLLSupported = ((*p_codec_info & A2DP_LHDC_LL_MASK) != 0)? true : false;
  //p_ie->isLLSupported = false;

  p_ie->hasFeatureLLAC = ((*p_codec_info & A2DP_LHDC_FEATURE_LLAC) != 0) ? true : false;
  p_codec_info += 1;

  p_ie->channelSplitMode = (*p_codec_info) & A2DP_LHDC_CH_SPLIT_MSK;

  //p_codec_info += 1;

  //p_ie->supportedBitrate = (*p_codec_info);


  p_ie->hasFeatureMETA = ((*p_codec_info & A2DP_LHDC_FEATURE_META) != 0) ? true : false;
  p_ie->hasFeatureMinBitrate = ((*p_codec_info & A2DP_LHDC_FEATURE_MIN_BR) != 0) ? true : false;
  p_ie->hasFeatureLARC = ((*p_codec_info & A2DP_LHDC_FEATURE_LARC) != 0) ? true : false;
  p_ie->hasFeatureLHDCV4 = ((*p_codec_info & A2DP_LHDC_FEATURE_LHDCV4) != 0) ? true : false;

  LOG_WARN(LOG_TAG, "%s:Has LL(%d) JAS(%d) AR(%d) META(%d) LLAC(%d) MBR(%d) LARC(%d) V4(%d)", __func__,
      p_ie->isLLSupported,
      p_ie->hasFeatureJAS,
      p_ie->hasFeatureAR,
      p_ie->hasFeatureMETA,
      p_ie->hasFeatureLLAC,
      p_ie->hasFeatureMinBitrate,
      p_ie->hasFeatureLARC,
      p_ie->hasFeatureLHDCV4);


    LOG_WARN(LOG_TAG, "%s: codec info = [0]:0x%x, [1]:0x%x, [2]:0x%x, [3]:0x%x, [4]:0x%x, [5]:0x%x, [6]:0x%x, [7]:0x%x, [8]:0x%x, [9]:0x%x, [10]:0x%x, [11]:0x%x",
            __func__, tmpInfo[0], tmpInfo[1], tmpInfo[2], tmpInfo[3], tmpInfo[4], tmpInfo[5], tmpInfo[6],
                        tmpInfo[7], tmpInfo[8], tmpInfo[9], tmpInfo[10], tmpInfo[11]);

  if (is_capability) return A2DP_SUCCESS;

  if (A2DP_BitsSet(p_ie->sampleRate) != A2DP_SET_ONE_BIT)
    return A2DP_BAD_SAMP_FREQ;

  save_codec_info (p_codec_Info_save);

  return A2DP_SUCCESS;
}

const char* A2DP_VendorCodecNameLhdcV3Sink(UNUSED_ATTR const uint8_t* p_codec_info) {
  return "LHDC V3 Sink";
}

bool A2DP_IsVendorSinkCodecValidLhdcV3(const uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, false) == A2DP_SUCCESS) ||
         (A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}


bool A2DP_IsVendorPeerSourceCodecValidLhdcV3(const uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, false) == A2DP_SUCCESS) ||
         (A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}


bool A2DP_IsVendorSinkCodecSupportedLhdcV3(const uint8_t* p_codec_info) {
  return (A2DP_CodecInfoMatchesCapabilityLhdcV3Sink(&a2dp_lhdcv3_sink_caps, p_codec_info,
                                             false) == A2DP_SUCCESS);
}

bool A2DP_IsPeerSourceCodecSupportedLhdcV3(const uint8_t* p_codec_info) {
  return (A2DP_CodecInfoMatchesCapabilityLhdcV3Sink(&a2dp_lhdcv3_sink_caps, p_codec_info,
                                             true) == A2DP_SUCCESS);
}

void A2DP_InitDefaultCodecLhdcV3Sink(uint8_t* p_codec_info) {
  LOG_DEBUG("%s: enter", __func__);
  if (A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &a2dp_lhdcv3_sink_default_config,
                        p_codec_info) != A2DP_SUCCESS) {
    LOG_ERROR("%s: A2DP_BuildInfoSbc failed", __func__);
  }
}


// Checks whether A2DP SBC codec configuration matches with a device's codec
// capabilities. |p_cap| is the SBC codec configuration. |p_codec_info| is
// the device's codec capabilities. |is_capability| is true if
// |p_codec_info| contains A2DP codec capability.
// Returns A2DP_SUCCESS if the codec configuration matches with capabilities,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityLhdcV3Sink(
    const tA2DP_LHDCV3_SINK_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_capability) {
  tA2DP_STATUS status;
  tA2DP_LHDCV3_SINK_CIE cfg_cie;

  /* parse configuration */
  status = A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR("%s: parsing failed %d", __func__, status);
    return status;
  }

  /* verify that each parameter is in range */

  LOG_DEBUG("%s: FREQ peer: 0x%x, capability 0x%x", __func__,
            cfg_cie.sampleRate, p_cap->sampleRate);

  LOG_DEBUG("%s: BIT_FMT peer: 0x%x, capability 0x%x", __func__,
            cfg_cie.bits_per_sample, p_cap->bits_per_sample);

  /* sampling frequency */
  if ((cfg_cie.sampleRate & p_cap->sampleRate) == 0) return A2DP_NS_SAMP_FREQ;

  /* bit per sample */
  if ((cfg_cie.bits_per_sample & p_cap->bits_per_sample) == 0) return A2DP_NS_CH_MODE;

  return A2DP_SUCCESS;
}

bool A2DP_VendorCodecTypeEqualsLhdcV3Sink(const uint8_t* p_codec_info_a,
                                    const uint8_t* p_codec_info_b) {
  tA2DP_LHDCV3_SINK_CIE lhdc_cie_a;
  tA2DP_LHDCV3_SINK_CIE lhdc_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoLhdcV3Sink(&lhdc_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return true;
}

bool A2DP_VendorCodecEqualsLhdcV3Sink(const uint8_t* p_codec_info_a,
                                const uint8_t* p_codec_info_b) {
  tA2DP_LHDCV3_SINK_CIE lhdc_cie_a;
  tA2DP_LHDCV3_SINK_CIE lhdc_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoLhdcV3Sink(&lhdc_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return (lhdc_cie_a.sampleRate == lhdc_cie_b.sampleRate) &&
         (lhdc_cie_a.bits_per_sample == lhdc_cie_b.bits_per_sample) &&
         /*(lhdc_cie_a.supportedBitrate == lhdc_cie_b.supportedBitrate) &&*/
         (lhdc_cie_a.isLLSupported == lhdc_cie_b.isLLSupported);
}


int A2DP_VendorGetTrackSampleRateLhdcV3Sink(const uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE lhdc_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  switch (lhdc_cie.sampleRate) {
    case A2DP_LHDC_SAMPLING_FREQ_44100:
      return 44100;
    case A2DP_LHDC_SAMPLING_FREQ_48000:
      return 48000;
    case A2DP_LHDC_SAMPLING_FREQ_88200:
      return 88200;
    case A2DP_LHDC_SAMPLING_FREQ_96000:
      return 96000;
  }

  return -1;
}

int A2DP_VendorGetSinkTrackChannelTypeLhdcV3(const uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE lhdc_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  return A2DP_LHDC_CHANNEL_MODE_STEREO;
}

int A2DP_VendorGetChannelModeCodeLhdcV3Sink(const uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE lhdc_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR("%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }
  return A2DP_LHDC_CHANNEL_MODE_STEREO;
}

bool A2DP_VendorGetPacketTimestampLhdcV3Sink(UNUSED_ATTR const uint8_t* p_codec_info,
                                       const uint8_t* p_data,
                                       uint32_t* p_timestamp) {
  // TODO: Is this function really codec-specific?
  *p_timestamp = *(const uint32_t*)p_data;
  return true;
}
/*
bool A2DP_VendorBuildCodecHeaderLhdcV3Sink(UNUSED_ATTR const uint8_t* p_codec_info,
                              BT_HDR* p_buf, uint16_t frames_per_packet) {
  uint8_t* p;

  p_buf->offset -= A2DP_SBC_MPL_HDR_LEN;
  p = (uint8_t*)(p_buf + 1) + p_buf->offset;
  p_buf->len += A2DP_SBC_MPL_HDR_LEN;
  A2DP_BuildMediaPayloadHeaderSbc(p, false, false, false,
                                  (uint8_t)frames_per_packet);

  return true;
}
*/
std::string A2DP_VendorCodecInfoStringLhdcV3Sink(const uint8_t* p_codec_info) {
  std::stringstream res;
  std::string field;
  tA2DP_STATUS a2dp_status;
  tA2DP_LHDCV3_SINK_CIE lhdc_cie;

  a2dp_status = A2DP_ParseInfoLhdcV3Sink(&lhdc_cie, p_codec_info, true);
  if (a2dp_status != A2DP_SUCCESS) {
    res << "A2DP_ParseInfoLhdcV3Sink fail: " << loghex(a2dp_status);
    return res.str();
  }

  res << "\tname: LHDC\n";

  // Sample frequency
  field.clear();
  AppendField(&field, (lhdc_cie.sampleRate == 0), "NONE");
  AppendField(&field, (lhdc_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_44100),
              "44100");
  AppendField(&field, (lhdc_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_48000),
              "48000");
  AppendField(&field, (lhdc_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_88200),
              "88200");
  AppendField(&field, (lhdc_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_96000),
              "96000");
  res << "\tsamp_freq: " << field << " (" << loghex(lhdc_cie.sampleRate)
      << ")\n";

  // Channel mode
  field.clear();
  AppendField(&field, 1,
             "Stereo");
  res << "\tch_mode: " << field << " (" << "Only support stereo."
      << ")\n";

  // bits per sample
  field.clear();
  AppendField(&field, (lhdc_cie.bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16),
              "16");
  AppendField(&field, (lhdc_cie.bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24),
              "24");
  res << "\tbits_depth: " << field << " bits (" << loghex((int)lhdc_cie.bits_per_sample)
      << ")\n";

  // Max data rate...
  field.clear();
  AppendField(&field, ((lhdc_cie.maxTargetBitrate & A2DP_LHDC_MAX_BIT_RATE_MASK) == A2DP_LHDC_MAX_BIT_RATE_900K),
              "900Kbps");
  AppendField(&field, ((lhdc_cie.maxTargetBitrate & A2DP_LHDC_MAX_BIT_RATE_MASK) == A2DP_LHDC_MAX_BIT_RATE_500K),
              "500Kbps");
  AppendField(&field, ((lhdc_cie.maxTargetBitrate & A2DP_LHDC_MAX_BIT_RATE_MASK) == A2DP_LHDC_MAX_BIT_RATE_400K),
              "400Kbps");
  res << "\tMax target-rate: " << field << " (" << loghex((lhdc_cie.maxTargetBitrate & A2DP_LHDC_MAX_BIT_RATE_MASK))
      << ")\n";

  // Version
  field.clear();
  AppendField(&field, (lhdc_cie.version == A2DP_LHDC_VER3),
              "LHDC V3");
  res << "\tversion: " << field << " (" << loghex(lhdc_cie.version)
      << ")\n";


  /*
  field.clear();
  AppendField(&field, 0, "NONE");
  AppendField(&field, 0,
              "Mono");
  AppendField(&field, 0,
              "Dual");
  AppendField(&field, 1,
              "Stereo");
  res << "\tch_mode: " << field << " (" << loghex(lhdc_cie.channelMode)
      << ")\n";
*/
  return res.str();
}

const tA2DP_DECODER_INTERFACE* A2DP_VendorGetDecoderInterfaceLhdcV3(
    const uint8_t* p_codec_info) {
  if (!A2DP_IsVendorSinkCodecValidLhdcV3(p_codec_info)) return NULL;

  return &a2dp_decoder_interface_lhdcv3;
}

bool A2DP_VendorAdjustCodecLhdcV3Sink(uint8_t* p_codec_info) {
  tA2DP_LHDCV3_SINK_CIE cfg_cie;

  // Nothing to do: just verify the codec info is valid
  if (A2DP_ParseInfoLhdcV3Sink(&cfg_cie, p_codec_info, true) != A2DP_SUCCESS)
    return false;

  return true;
}

btav_a2dp_codec_index_t A2DP_VendorSinkCodecIndexLhdcV3(
    UNUSED_ATTR const uint8_t* p_codec_info) {
  return BTAV_A2DP_CODEC_INDEX_SINK_LHDCV3;
}

const char* A2DP_VendorCodecIndexStrLhdcV3Sink(void) { return "LHDC V3 SINK"; }

bool A2DP_VendorInitCodecConfigLhdcV3Sink(AvdtpSepConfig* p_cfg) {
  LOG_DEBUG("%s: enter", __func__);
  if (A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &a2dp_lhdcv3_sink_caps,
                        p_cfg->codec_info) != A2DP_SUCCESS) {
    return false;
  }

  return true;
}

UNUSED_ATTR static void build_codec_config(const tA2DP_LHDCV3_SINK_CIE& config_cie,
                                           btav_a2dp_codec_config_t* result) {
  if (config_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_44100)
    result->sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
  if (config_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_48000)
    result->sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
  if (config_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_88200)
    result->sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_88200;
  if (config_cie.sampleRate & A2DP_LHDC_SAMPLING_FREQ_96000)
    result->sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_96000;

  result->bits_per_sample = config_cie.bits_per_sample;

  result->channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
}



A2dpCodecConfigLhdcV3Sink::A2dpCodecConfigLhdcV3Sink(
    btav_a2dp_codec_priority_t codec_priority)
    : A2dpCodecConfigLhdcV3Base(BTAV_A2DP_CODEC_INDEX_SINK_LHDCV3,
                             A2DP_VendorCodecIndexStrLhdcV3Sink(), codec_priority,
                             false) {}

A2dpCodecConfigLhdcV3Sink::~A2dpCodecConfigLhdcV3Sink() {}

bool A2dpCodecConfigLhdcV3Sink::init() {
  if (!isValid()) return false;

  // Load the decoder
  if (!A2DP_VendorLoadDecoderLhdcV3()) {
    LOG_ERROR("%s: cannot load the decoder", __func__);
    return false;
  }

  return true;
}

bool A2dpCodecConfigLhdcV3Sink::useRtpHeaderMarkerBit() const {
  // TODO: This method applies only to Source codecs
  return false;
}

bool A2dpCodecConfigLhdcV3Sink::updateEncoderUserConfig(
    UNUSED_ATTR const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params,
    UNUSED_ATTR bool* p_restart_input, UNUSED_ATTR bool* p_restart_output,
    UNUSED_ATTR bool* p_config_updated) {
  // TODO: This method applies only to Source codecs
  return false;
}

uint64_t A2dpCodecConfigLhdcV3Sink::encoderIntervalMs() const {
  // TODO: This method applies only to Source codecs
  return 0;
}

int A2dpCodecConfigLhdcV3Sink::getEffectiveMtu() const {
  // TODO: This method applies only to Source codecs
  return 0;
}


static bool select_best_sample_rate(uint8_t sampleRate,
    tA2DP_LHDCV3_SINK_CIE* p_result,
    btav_a2dp_codec_config_t* p_codec_config) {
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_96000) {
    p_result->sampleRate = A2DP_LHDC_SAMPLING_FREQ_96000;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
    return true;
  }
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_48000) {
    p_result->sampleRate = A2DP_LHDC_SAMPLING_FREQ_48000;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    return true;
  }
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_44100) {
    p_result->sampleRate = A2DP_LHDC_SAMPLING_FREQ_44100;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    return true;
  }
  return false;
  }

static bool select_best_bits_per_sample(
    btav_a2dp_codec_bits_per_sample_t bits_per_sample, tA2DP_LHDCV3_SINK_CIE* p_result,
    btav_a2dp_codec_config_t* p_codec_config) {
  if (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24) {
     p_codec_config->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
    p_result->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
    return true;
  }
  if (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16) {
    p_codec_config->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
    p_result->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
    return true;
  }
  return false;
}


bool A2dpCodecConfigLhdcV3Base::setCodecConfig(const uint8_t* p_peer_codec_info, bool is_capability,
                      uint8_t* p_result_codec_config) {
    is_source_ = false;

    std::lock_guard<std::recursive_mutex> lock(codec_mutex_);
    tA2DP_LHDCV3_SINK_CIE peer_info_cie;    //codec info of peer (source)
    tA2DP_LHDCV3_SINK_CIE result_config_cie;
    uint8_t sampleRate;
    btav_a2dp_codec_bits_per_sample_t bits_per_sample;
    const tA2DP_LHDCV3_SINK_CIE* p_a2dp_lhdcv3_caps = NULL;


  LOG_WARN(LOG_TAG, "%s: is_capability = %d", __func__, is_capability);

  // Save the internal state
  btav_a2dp_codec_config_t saved_codec_config = codec_config_;
  btav_a2dp_codec_config_t saved_codec_capability = codec_capability_;
  btav_a2dp_codec_config_t saved_codec_selectable_capability =
      codec_selectable_capability_;
  btav_a2dp_codec_config_t saved_codec_user_config = codec_user_config_;
  btav_a2dp_codec_config_t saved_codec_audio_config = codec_audio_config_;
  uint8_t saved_ota_codec_config[AVDT_CODEC_SIZE];
  uint8_t saved_ota_codec_peer_capability[AVDT_CODEC_SIZE];
  uint8_t saved_ota_codec_peer_config[AVDT_CODEC_SIZE];
  memcpy(saved_ota_codec_config, ota_codec_config_, sizeof(ota_codec_config_));
  memcpy(saved_ota_codec_peer_capability, ota_codec_peer_capability_,
         sizeof(ota_codec_peer_capability_));
  memcpy(saved_ota_codec_peer_config, ota_codec_peer_config_,
         sizeof(ota_codec_peer_config_));

  tA2DP_STATUS status =
      A2DP_ParseInfoLhdcV3Sink(&peer_info_cie, p_peer_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: can't parse peer's Sink capabilities: error = %d",
              __func__, status);
    goto fail;
  }

  if (peer_info_cie.hasFeatureLHDCV4 && peer_info_cie.hasFeatureLLAC) {
    //LHDCV4 + LLAC
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v4_config;
    LOG_WARN(LOG_TAG, "%s: LHDCV4 + LLAC", __func__);
  } else if (peer_info_cie.hasFeatureLHDCV4 && !peer_info_cie.hasFeatureLLAC) {
    //LHDCV4 Only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v3_config;
    LOG_WARN(LOG_TAG, "%s: LHDCV4 Only", __func__);
  } else if (!peer_info_cie.hasFeatureLHDCV4 && peer_info_cie.hasFeatureLLAC) {
    //LLAC Only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v4_config;
    LOG_WARN(LOG_TAG, "%s: LLAC Only", __func__);
  } else if (!peer_info_cie.hasFeatureLHDCV4 && !peer_info_cie.hasFeatureLLAC) {
    //LHDC V3 only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v3_config;
    LOG_WARN(LOG_TAG, "%s: LHDC V3 only", __func__);
  }

  //
  // Build the preferred configuration
  //
  memset(&result_config_cie, 0, sizeof(result_config_cie));
  result_config_cie.vendorId = p_a2dp_lhdcv3_caps->vendorId;
  result_config_cie.codecId = p_a2dp_lhdcv3_caps->codecId;
  result_config_cie.version = p_a2dp_lhdcv3_caps->version;

  //
  // Select the sample frequency
  //
  sampleRate = p_a2dp_lhdcv3_caps->sampleRate & peer_info_cie.sampleRate;
  LOG_DEBUG(LOG_TAG, "%s: sampleRate local:0x%x peer:0x%x matched:0x%x", __func__,
      p_a2dp_lhdcv3_caps->sampleRate, peer_info_cie.sampleRate, sampleRate);

  codec_config_.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
  // Select the sample frequency if there is no user preference
  do {
    // No user preference - use the best match
    if (select_best_sample_rate(sampleRate, &result_config_cie,
                                &codec_config_)) {
      LOG_DEBUG(LOG_TAG, "%s: select best sample rate(best):0x%x", __func__,
          result_config_cie.sampleRate);
      break;
    }
  } while (false);
  if (codec_config_.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_NONE) {
    LOG_ERROR(LOG_TAG,
              "%s: cannot match sample frequency: local caps = 0x%x "
              "peer info = 0x%x",
              __func__, p_a2dp_lhdcv3_caps->sampleRate, peer_info_cie.sampleRate);
    goto fail;
  }
  LOG_WARN(LOG_TAG, "%s: final sample rate = 0x%02X", __func__,
      result_config_cie.sampleRate);

  //
  // Select the bits per sample
  //
  // NOTE: this information is NOT included in the LHDC A2DP codec description
  // that is sent OTA.
  bits_per_sample = p_a2dp_lhdcv3_caps->bits_per_sample & peer_info_cie.bits_per_sample;
  LOG_DEBUG(LOG_TAG, "%s: bits_per_sample src:0x%02x sink:0x%02x matched:0x%02x", __func__,
      p_a2dp_lhdcv3_caps->bits_per_sample, peer_info_cie.bits_per_sample, bits_per_sample);

  codec_config_.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
  // Select the bits per sample if there is no user preference
  do {
    // No user preference - use the best match
    if (select_best_bits_per_sample(bits_per_sample,
                                    &result_config_cie, &codec_config_)) {
      LOG_DEBUG(LOG_TAG, "%s: select best bits_per_sample(best):0x%x",
          __func__, result_config_cie.bits_per_sample);
      break;
    }
  } while (false);
  if (codec_config_.bits_per_sample == BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE) {
    LOG_ERROR(LOG_TAG,
              "%s: cannot match bits per sample", __func__);
    goto fail;
  }
  LOG_WARN(LOG_TAG, "%s: final bits per sample = 0x%02X", __func__,
      result_config_cie.bits_per_sample);

  /**
  *LHDC V4 modify
  */

  /*******************************************
   * Update Feature/Capabilities: LLAC
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureLLAC = p_a2dp_lhdcv3_caps->hasFeatureLLAC;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureLLAC = 0x%02X", __func__,
      result_config_cie.hasFeatureLLAC);

  /*******************************************
   * Update Feature/Capabilities: LHDCV4
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureLHDCV4 = p_a2dp_lhdcv3_caps->hasFeatureLHDCV4;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureLHDCV4 = 0x%02X", __func__,
      result_config_cie.hasFeatureLHDCV4);

  /*******************************************
   * Update Feature/Capabilities: JAS
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureJAS = p_a2dp_lhdcv3_caps->hasFeatureJAS;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureJAS = 0x%02X", __func__,
      result_config_cie.hasFeatureJAS);

  /*******************************************
   * Update Feature/Capabilities: AR
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureAR = p_a2dp_lhdcv3_caps->hasFeatureAR;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureAR = 0x%02X", __func__,
      result_config_cie.hasFeatureAR);

  /*******************************************
   * Update Feature/Capabilities: META
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureMETA = p_a2dp_lhdcv3_caps->hasFeatureMETA;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureMETA = 0x%02X", __func__,
      result_config_cie.hasFeatureMETA);

  /*******************************************
   * Update Feature/Capabilities: MBR
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureMinBitrate = p_a2dp_lhdcv3_caps->hasFeatureMinBitrate;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureMinBitrate = 0x%02X", __func__,
      result_config_cie.hasFeatureMinBitrate);

  /*******************************************
   * Update Feature/Capabilities: LARC
   * to A2DP specifics
   *******************************************/
  result_config_cie.hasFeatureLARC = p_a2dp_lhdcv3_caps->hasFeatureLARC;
  LOG_WARN(LOG_TAG, "%s: final hasFeatureLARC = 0x%02X", __func__,
      result_config_cie.hasFeatureLARC);

  //
  // Copy the codec-specific fields if they are not zero
  //
  if (codec_user_config_.codec_specific_1 != 0)
    codec_config_.codec_specific_1 = codec_user_config_.codec_specific_1;
  if (codec_user_config_.codec_specific_2 != 0)
    codec_config_.codec_specific_2 = codec_user_config_.codec_specific_2;
  if (codec_user_config_.codec_specific_3 != 0)
    codec_config_.codec_specific_3 = codec_user_config_.codec_specific_3;
  if (codec_user_config_.codec_specific_4 != 0)
    codec_config_.codec_specific_4 = codec_user_config_.codec_specific_4;

  /* Setup final nego result codec config to peer */
  if (int ret = A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
                         p_result_codec_config) != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG,"%s: A2DP_BuildInfoLhdcV3 fail(0x%x)", __func__, ret);
    goto fail;
  }

  // Create a local copy of the peer codec capability, and the
  // result codec config.
  if (is_capability) {
    status = A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &peer_info_cie,
                                ota_codec_peer_capability_);
  } else {
    status = A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &peer_info_cie,
                                ota_codec_peer_config_);
  }
  CHECK(status == A2DP_SUCCESS);

  status = A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
                              ota_codec_config_);
  CHECK(status == A2DP_SUCCESS);

  LOG_WARN(LOG_TAG, "%s: done", __func__);
  return true;

fail:
  // Restore the internal state
  codec_config_ = saved_codec_config;
  codec_capability_ = saved_codec_capability;
  codec_selectable_capability_ = saved_codec_selectable_capability;
  codec_user_config_ = saved_codec_user_config;
  codec_audio_config_ = saved_codec_audio_config;
  memcpy(ota_codec_config_, saved_ota_codec_config, sizeof(ota_codec_config_));
  memcpy(ota_codec_peer_capability_, saved_ota_codec_peer_capability,
         sizeof(ota_codec_peer_capability_));
  memcpy(ota_codec_peer_config_, saved_ota_codec_peer_config,
         sizeof(ota_codec_peer_config_));

  LOG_WARN(LOG_TAG, "%s: success", __func__);
  return false;
}
 


bool A2dpCodecConfigLhdcV3Base::setPeerCodecCapabilities(
      const uint8_t* p_peer_codec_capabilities) {
  is_source_ = false;

  
std::lock_guard<std::recursive_mutex> lock(codec_mutex_);
  tA2DP_LHDCV3_SINK_CIE peer_info_cie;
  uint8_t sampleRate;
  uint8_t bits_per_sample;
  const tA2DP_LHDCV3_SINK_CIE* p_a2dp_lhdcv3_caps = NULL;

  LOG_WARN(LOG_TAG, "%s: enter", __func__);

  // Save the internal state
  btav_a2dp_codec_config_t saved_codec_selectable_capability =
  codec_selectable_capability_;
  uint8_t saved_ota_codec_peer_capability[AVDT_CODEC_SIZE];
  memcpy(saved_ota_codec_peer_capability, ota_codec_peer_capability_,
         sizeof(ota_codec_peer_capability_));

  tA2DP_STATUS status =
  A2DP_ParseInfoLhdcV3Sink(&peer_info_cie, p_peer_codec_capabilities, true);
  if (status != A2DP_SUCCESS) {
      LOG_ERROR(LOG_TAG, "%s: can't parse peer's capabilities: error = %d",
                __func__, status);
      goto fail;
  }

  if (peer_info_cie.hasFeatureLHDCV4 && peer_info_cie.hasFeatureLLAC) {
    //LHDCV4 + LLAC
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v4_config;
    LOG_WARN(LOG_TAG, "%s: LHDCV4 + LLAC", __func__);
  } else if (peer_info_cie.hasFeatureLHDCV4 && !peer_info_cie.hasFeatureLLAC) {
    //LHDCV4 Only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v3_config;
    LOG_WARN(LOG_TAG, "%s: LHDCV4 only", __func__);
  } else if (!peer_info_cie.hasFeatureLHDCV4 && peer_info_cie.hasFeatureLLAC) {
    //LLAC Only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v4_config;
    LOG_WARN(LOG_TAG, "%s: LLAC only", __func__);
  } else if (!peer_info_cie.hasFeatureLHDCV4 && !peer_info_cie.hasFeatureLLAC) {
    //LHDC V3 only
    p_a2dp_lhdcv3_caps = &a2dp_lhdcv3_sink_v3_config;
    LOG_WARN(LOG_TAG, "%s: LHDC V3 only", __func__);
  }

  // Compute the selectable capability - bits per sample
  //codec_selectable_capability_.bits_per_sample =
  //p_a2dp_lhdcv3_caps->bits_per_sample;
  bits_per_sample = p_a2dp_lhdcv3_caps->bits_per_sample & peer_info_cie.bits_per_sample;
  if (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16) {
      codec_selectable_capability_.bits_per_sample |= BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
  }
  if (bits_per_sample & BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24) {
      codec_selectable_capability_.bits_per_sample |= BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24;
  }

  // Compute the selectable capability - sample rate
  sampleRate = p_a2dp_lhdcv3_caps->sampleRate & peer_info_cie.sampleRate;
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_44100) {
      codec_selectable_capability_.sample_rate |=
      BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
  }
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_48000) {
      codec_selectable_capability_.sample_rate |=
      BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
  }
  if (sampleRate & A2DP_LHDC_SAMPLING_FREQ_96000) {
      codec_selectable_capability_.sample_rate |=
      BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
  }

  // Compute the selectable capability - channel mode
  codec_selectable_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;

  status = A2DP_BuildInfoLhdcV3Sink(AVDT_MEDIA_TYPE_AUDIO, &peer_info_cie,
                              ota_codec_peer_capability_);
  CHECK(status == A2DP_SUCCESS);

  LOG_WARN(LOG_TAG, "%s: sampleRate[local=%02X peer=%02X]:%02X", __func__,
      p_a2dp_lhdcv3_caps->sampleRate, peer_info_cie.sampleRate, sampleRate);

  LOG_WARN(LOG_TAG, "%s: bitsPerSample[local=%02X peer=%02X]:%02X", __func__,
      p_a2dp_lhdcv3_caps->bits_per_sample, peer_info_cie.bits_per_sample, bits_per_sample);

  LOG_WARN(LOG_TAG, "%s: success!", __func__);
   return true;

fail:
  // Restore the internal state
  codec_selectable_capability_ = saved_codec_selectable_capability;
  memcpy(ota_codec_peer_capability_, saved_ota_codec_peer_capability,
         sizeof(ota_codec_peer_capability_));
  LOG_WARN(LOG_TAG, "%s: fail", __func__);
  return false;
}
 
 

