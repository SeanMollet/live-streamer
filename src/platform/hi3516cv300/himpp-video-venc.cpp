/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * himpp-video-venc.cpp
 * Copyright (C) 2015 Watson Xu <xuhuashan@gmail.com>
 *
 * live-streamer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * live-streamer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ev++.h>
#include <mpi_sys.h>
#include <mpi_venc.h>
#include <mpi_region.h>

#include <himpp-common.h>
#include <himpp-video-region.h>
#include "himpp-video-venc.h"

extern ev::default_loop mainloop;

HimppVencChan::HimppVencChan
(HimppVideoElement* source, VideoEncodingType encoding, VENC_CHN chnid)
  : VideoElement(VIDEO_ELEMENT(source)),
    HimppVideoElement(source),
    _chnid(chnid),
    _encoding(encoding),
    _h264profile(HIGH),
		_h265profile(HEVC_MAIN),
    _rcmode(VBR),
    _resolution(source->resolution()),
    _framerate(source->framerate()),
    _bitrate(2048),
    //_gop(_framerate * 2),
    _gop(_framerate),
    _min_qp(16),
    _max_qp(51),
	_refmode(1, 0, true),
	_intrarefresh(false, false, 11, 51),
	_io(mainloop)
{
	_crop_cfg.bEnable = HI_FALSE;
	_crop_cfg.stRect.u32Width = 0;
	_crop_cfg.stRect.u32Height = 0;
	_crop_cfg.stRect.s32X = 0;
	_crop_cfg.stRect.s32Y = 0;

	_mpp_chn.enModId = HI_ID_VENC;
	_mpp_chn.s32DevId = 0;
	_mpp_chn.s32ChnId = chnid;
}

HimppVencChan::~HimppVencChan()
{
}

VideoEncodingType HimppVencChan::encoding()
{
	return _encoding;
}

uint32_t HimppVencChan::bitrate()
{
	return _bitrate;
}

void HimppVencChan::attach(StreamSink* sink)
{
	VideoStreamSource::attach(sink);
}

void HimppVencChan::detach(StreamSink* sink)
{
	VideoStreamSource::detach(sink);
}

void HimppVencChan::play()
{
	enable();
}

void HimppVencChan::stop()
{
	disable();
}

void HimppVencChan::resume()
{
	_io.set<HimppVencChan, &HimppVencChan::watch_handler>(this);
	_io.set(HI_MPI_VENC_GetFd(channelId()), ev::READ);
	_io.start();
}

void HimppVencChan::pause()
{
	_io.stop();
}

VideoEncodingType HimppVencChan::getEncoding()
{
	return _encoding;
}

void HimppVencChan::watch_handler(ev::io &w, int revents)
{
	if (!(revents & ev::READ))
		return;

	HI_S32 s32Ret;
	int chnid = channelId();

	VENC_CHN_STAT_S stChnStat;
	s32Ret = HI_MPI_VENC_Query(chnid, &stChnStat);
	if (s32Ret != HI_SUCCESS || stChnStat.u32CurPacks == 0)
		return;

	VENC_PACK_S stPack[stChnStat.u32CurPacks];
	VENC_STREAM_S stStream = {
		.pstPack = stPack,
		.u32PackCount = stChnStat.u32CurPacks,
		.u32Seq = 0
	};

	s32Ret = HI_MPI_VENC_GetStream(chnid, &stStream, 0);
	if (s32Ret != HI_SUCCESS) {
		HIMPP_PRINT("Get video stream %d failed [%#x]\n", chnid, s32Ret);
		return;
	}

	StreamBuffer *sbuf = NULL;
	JPEGStreamBuffer jpegbuffer;
	H264StreamBuffer h264buffer;
	H265StreamBuffer h265buffer;

	switch (encoding()) {
	case H264:
		h264buffer.keyframe = (stStream.stH264Info.enRefType == BASE_IDRSLICE);
		sbuf = &h264buffer;
		break;
	case H265:
		h265buffer.keyframe = (stStream.stH265Info.enRefType == BASE_IDRSLICE);
		sbuf = &h265buffer;
		break;		
	case JPEG:
	case MJPEG:
		jpegbuffer.qfactor = stStream.stJpegInfo.u32Qfactor;
		jpegbuffer.width = resolution().width() >> 3;
		jpegbuffer.height = resolution().height() >> 3;
		sbuf = &jpegbuffer;
		break;
	default:
		HIMPP_PRINT("Unsupported video encoding %d\n", encoding());
		break;
	}

	if (sbuf) {
		StreamBuffer::Pack stream_data_pack[stStream.u32PackCount];
		memset(&stream_data_pack, 0, sizeof(stream_data_pack));
		sbuf->pack_count = stStream.u32PackCount;
		sbuf->pack = stream_data_pack;
		gettimeofday(&sbuf->tstamp, NULL);

		for (int i = 0; i < (int)stStream.u32PackCount; i++) {
			VENC_PACK_S *pstPack = &stStream.pstPack[i];
			sbuf->pack[i].addr = pstPack->pu8Addr + pstPack->u32Offset;
			sbuf->pack[i].len = pstPack->u32Len - pstPack->u32Offset;
		}

		streamData(sbuf);
	}

	s32Ret = HI_MPI_VENC_ReleaseStream(chnid, &stStream);
	if (s32Ret != HI_SUCCESS) {
		HIMPP_PRINT("Release video stream %d failed [%#x]\n", chnid, s32Ret);
	}
}

void HimppVencChan::setH264Profile(H264Profile profile)
{
	if (is_enabled()) {
		H264Profile oldval = _h264profile;
		doDisableElement();
		try {
			_h264profile = profile;
			doEnableElement();
		} catch (IpcamError& e) {
			_h264profile = oldval;
			doEnableElement();
			throw e;
		}
	}
	_h264profile = profile;
}

H264Profile HimppVencChan::getH264Profile()
{
	return _h264profile;
}

void HimppVencChan::setH265Profile(H265Profile profile)
{
	if (is_enabled()) {
		H265Profile oldval = _h265profile;
		doDisableElement();
		try {
			_h265profile = profile;
			doEnableElement();
		} catch (IpcamError& e) {
			_h265profile = oldval;
			doEnableElement();
			throw e;
		}
	}
	_h265profile = profile;
}

H265Profile HimppVencChan::getH265Profile()
{
	return _h265profile;
}


void HimppVencChan::setRcMode(RateCtrlMode mode)
{
	if (is_enabled()) {
		RateCtrlMode oldval = _rcmode;
		doDisableElement();
		try {
			_rcmode = mode;
			doEnableElement();
		} catch (IpcamError& e) {
			_rcmode = oldval;
			doEnableElement();
			throw e;
		}
	}
	_rcmode = mode;
}

RateCtrlMode HimppVencChan::getRcMode()
{
	return _rcmode;
}

void HimppVencChan::setBitrate(uint32_t value)
{
	if (is_enabled()) {
		VENC_CHN_ATTR_S attr;
		HI_S32 retval;

		if ((retval = HI_MPI_VENC_GetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
			throw IpcamError("Failed to get venc chan attr");
		}

		switch (_rcmode) {
			case CBR: 
				attr.stRcAttr.stAttrH264Cbr.u32BitRate = value; 
				attr.stRcAttr.stAttrH265Cbr.u32BitRate = value; 
				break;
			case VBR: 
				attr.stRcAttr.stAttrH264AVbr.u32MaxBitRate = value;
				attr.stRcAttr.stAttrH265AVbr.u32MaxBitRate = value;
				break;
			default: throw IpcamError("Cannot change Bitrate in current rc mode"); break;
		}

		if ((retval = HI_MPI_VENC_SetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
			throw IpcamError("Failed to set Bitrate");
		}
	}
	_bitrate = value;
}

uint32_t HimppVencChan::getBitrate()
{
	return _bitrate;
}

void HimppVencChan::setGovLength(uint32_t value)
{
	if (is_enabled()) {
		VENC_CHN_ATTR_S attr;
		HI_S32 retval;

		if ((retval = HI_MPI_VENC_GetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
			throw IpcamError("Failed to get venc chan attr");
		}

		switch (_rcmode) {
		case CBR: 
			attr.stRcAttr.stAttrH264Cbr.u32Gop = value; 
			attr.stRcAttr.stAttrH265Cbr.u32Gop = value; 
			break;
		case VBR: 
			attr.stRcAttr.stAttrH264AVbr.u32Gop = value;
			attr.stRcAttr.stAttrH265AVbr.u32Gop = value;
			break;
		case FIXQP: 
			attr.stRcAttr.stAttrH264FixQp.u32Gop = value;
			attr.stRcAttr.stAttrH265FixQp.u32Gop = value;
			break;
		default: throw IpcamError("Cannot change GovLength in current rc mode"); break;
		}

		if ((retval = HI_MPI_VENC_SetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
			throw IpcamError("Failed to set GovLength");
		}
	}
	_gop = value;
}

uint32_t HimppVencChan::getGovLength()
{
	return _gop;
}

void HimppVencChan::setMinQP(uint32_t value)
{
	if (is_enabled()) {
		VENC_RC_PARAM_S param;
		HI_S32 retval;

		if ((retval = HI_MPI_VENC_GetRcParam(_chnid, &param)) != HI_SUCCESS) {
			throw IpcamError("Failed GetRcParam");
		}

		switch (_rcmode) {
		case CBR:
			param.stParamH264Cbr.u32MinQp = value;
			param.stParamH264Cbr.u32MinIQp = value;
			param.stParamH265Cbr.u32MinQp = value;
			param.stParamH265Cbr.u32MinIQp = value;
			break;
		case VBR:
			param.stParamH264AVbr.u32MinQp = value;
			param.stParamH264AVbr.u32MinIQp = MIN2(value, param.stParamH264AVbr.u32MaxStillQP);
			param.stParamH265AVbr.u32MinQp = value;
			param.stParamH265AVbr.u32MinIQp = MIN2(value, param.stParamH265AVbr.u32MaxStillQP);
			break;
		default:
			throw IpcamError("Cannot change MinQP in current rc mode"); break;
		}

		if ((retval = HI_MPI_VENC_SetRcParam(_chnid, &param)) != HI_SUCCESS) {
			throw IpcamError("Failed to set MinQP");
		}
	}
	_min_qp = value;
}

uint32_t HimppVencChan::getMinQP()
{
	return _min_qp;
}

void HimppVencChan::setMaxQP(uint32_t value)
{
	if (is_enabled()) {
		VENC_RC_PARAM_S param;
		HI_S32 retval;

		if ((retval = HI_MPI_VENC_GetRcParam(_chnid, &param)) != HI_SUCCESS) {
			throw IpcamError("Failed GetRcParam");
		}

		switch (_rcmode) {
		case CBR:
			param.stParamH264Cbr.u32MaxQp = value;
			param.stParamH264Cbr.u32MaxIQp = value;
			param.stParamH265Cbr.u32MaxQp = value;
			param.stParamH265Cbr.u32MaxIQp = value;
			break;
		case VBR:
			param.stParamH264AVbr.u32MaxQp = value;
			param.stParamH264AVbr.u32MaxIQp = value;
			param.stParamH265AVbr.u32MaxQp = value;
			param.stParamH265Vbr.u32MaxIprop = value;			
			break;
		default:
			throw IpcamError("Cannot change MaxQP in current rc mode"); break;
		}

		if ((retval = HI_MPI_VENC_SetRcParam(_chnid, &param)) != HI_SUCCESS) {
			throw IpcamError("Failed to set MaxQP");
		}
	}
	_max_qp = value;
}

uint32_t HimppVencChan::getMaxQP()
{
	return _max_qp;
}

void HimppVencChan::setFrameRefMode(FrameRefMode value)
{
	if (is_enabled()) {
		VENC_PARAM_REF_S stRefParam;
		HI_S32 retval;
		if ((retval = HI_MPI_VENC_GetRefParam(_chnid, &stRefParam)) != HI_SUCCESS) {
			throw IpcamError("Failed GetRefParam");
		}

		stRefParam.u32Base = value.Base;
		stRefParam.u32Enhance = value.Enhanced;
		stRefParam.bEnablePred = (HI_BOOL)value.EnablePred;

		if ((retval = HI_MPI_VENC_SetRefParam(_chnid, &stRefParam)) != HI_SUCCESS) {
			throw IpcamError("Failed SetRefparam");
		}
	}
	_refmode = value;
}

FrameRefMode HimppVencChan::getFrameRefMode()
{
	if (is_enabled()) {
		VENC_PARAM_REF_S stRefParam;
		HI_S32 s32Ret;
		if ((s32Ret = HI_MPI_VENC_GetRefParam(_chnid, &stRefParam)) == HI_SUCCESS) {
			_refmode = FrameRefMode(stRefParam.u32Base, stRefParam.u32Enhance, stRefParam.bEnablePred);
		}
	}
	return _refmode;
}

void HimppVencChan::setIntraRefresh(IntraRefreshParam value)
{
	if (is_enabled()) {
		VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
		HI_S32 retval;
		if ((retval = HI_MPI_VENC_GetIntraRefresh(_chnid, &stIntraRefresh)) != HI_SUCCESS) {
			throw IpcamError("Failed GetIntraRefresh");
		}

		stIntraRefresh.bRefreshEnable = (HI_BOOL)value.EnableRefresh;
		stIntraRefresh.bISliceEnable = (HI_BOOL)value.EnableISlice;
		stIntraRefresh.u32RefreshLineNum = value.RefreshLineNum;
		stIntraRefresh.u32ReqIQp = value.ReqIQp;

		if ((retval = HI_MPI_VENC_SetIntraRefresh(_chnid, &stIntraRefresh)) != HI_SUCCESS) {
			throw IpcamError("Failed SetIntraRefresh");
		}
	}
	_intrarefresh = value;
}

IntraRefreshParam HimppVencChan::getIntraRefresh()
{
	if (is_enabled()) {
		VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
		HI_S32 s32Ret;
		if ((s32Ret = HI_MPI_VENC_GetIntraRefresh(_chnid, &stIntraRefresh)) == HI_SUCCESS) {
			_intrarefresh = IntraRefreshParam (stIntraRefresh.bRefreshEnable, stIntraRefresh.bISliceEnable, stIntraRefresh.u32RefreshLineNum, stIntraRefresh.u32ReqIQp);
		}
	}
	return _intrarefresh;
}

void HimppVencChan::requestIDR()
{
	if (is_enabled()) {
		HI_S32 s32Ret = HI_MPI_VENC_RequestIDR(_chnid, HI_TRUE);
		if (s32Ret != HI_SUCCESS) {
			HIMPP_PRINT("HI_MPI_VENC_RequestIDR %d failed [%#x]\n",
			            _chnid, s32Ret);
		}
	}
}

VideoOSD* HimppVencChan::CreateOSD()
{
	RGN_HANDLE handle = himpp_region_allocator.allocHandle();
	if (handle == (RGN_HANDLE)-1)
		throw IpcamError("Failed to allocate region handle");

	HimppVideoRegion* region(new HimppVideoRegion(this, handle));
	if (region == nullptr) {
		himpp_region_allocator.freeHandle(handle);
		throw IpcamError("Failed to create video osd");
	}

	_osds.insert(SOFT_RENDER_VIDEO_OSD(region));

	if (is_enabled()) {
		region->enable();
	}

	return region;
}

void HimppVencChan::DeleteOSD(VideoOSD* osd)
{
	_osds.erase(SOFT_RENDER_VIDEO_OSD(osd));
}

MPP_CHN_S* HimppVencChan::bindSource()
{
	return &_mpp_chn;
}

Resolution HimppVencChan::resolution()
{
	return _resolution;
}

uint32_t HimppVencChan::framerate()
{
	return _framerate;
}

// Calculate the crop configuration
static void calc_crop_cfg(Resolution& in, Resolution& out, VENC_CROP_CFG_S& crop_cfg)
{
	RECT_S &rect = crop_cfg.stRect;
	if (in.width() * out.height() > out.width() * in.height()) {
		// crop width
		crop_cfg.bEnable = HI_TRUE;
		rect.u32Height = in.height();
		rect.u32Width = out.width() * in.height() / out.height();
		rect.s32X = ((in.width() - rect.u32Width) / 2) & 0xFFFFFFF0;
		rect.s32Y = 0;
	}
	else if (in.width() * out.height() < out.width() * in.height()) {
		// crop height
		crop_cfg.bEnable = HI_TRUE;
		rect.u32Width = in.width();
		rect.u32Height = out.height() * in.width() / out.width();
		rect.s32X = 0;
		rect.s32Y = (in.height() - rect.u32Height) / 2;
	}
	else {
		// crop is not necessary
		crop_cfg.bEnable = HI_FALSE;
		rect.u32Width = 0;
		rect.u32Height = 0;
		rect.s32X = 0;
		rect.s32Y = 0;
	}
}

void HimppVencChan::setResolution(Resolution value)
{
	Resolution in = HIMPP_VIDEO_ELEMENT(source())->resolution();
	// Sanity check
	if ((value.width() > in.width()) || (value.height() > in.height())) {
		throw IpcamError("Resolution must not larger than input");
	}

	if (is_enabled()) {
		Resolution oldres = _resolution;
		doDisableElement();
		try {
			_resolution = value;
			doEnableElement();
		} catch (IpcamError& e) {
			_resolution = oldres;
			doEnableElement();
			throw e;
		}
	}
	_resolution = value;
}

Resolution HimppVencChan::getResolution()
{
	return _resolution;
}

void HimppVencChan::setFramerate(uint32_t value)
{
	uint32_t newgop = ((_gop + _framerate / 2) / _framerate) * value;
	if (is_enabled()) {
		uint32_t oldval = _framerate;
		uint32_t oldgop = _gop;
		doDisableElement();
		try {
			_framerate = value;
			_gop = newgop;
			doEnableElement();
		} catch (IpcamError& e) {
			_framerate = oldval;
			_gop = oldgop;
			doEnableElement();
			throw e;
		}
	}
	_framerate = value;
	_gop = newgop;
}

uint32_t HimppVencChan::getFramerate()
{
	return _framerate;
}

void HimppVencChan::prepareRcAttr(VENC_RC_ATTR_S &attr)
{
	HI_U32 stattime;
	stattime = _gop / _framerate;
	stattime = stattime > 0 ? stattime : 1;
	uint32_t ifr = VIDEO_ELEMENT(source())->framerate();

	switch (_encoding) {
	case H264:
		switch (_rcmode) {
		case CBR:
			attr.enRcMode = VENC_RC_MODE_H264CBR;
			attr.stAttrH264Cbr.u32Gop = _gop;
			attr.stAttrH264Cbr.u32StatTime = stattime;
			attr.stAttrH264Cbr.u32SrcFrmRate = ifr;
			attr.stAttrH264Cbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH264Cbr.u32BitRate = _bitrate;
			attr.stAttrH264Cbr.u32FluctuateLevel = 0;
			break;
		case VBR:
			attr.enRcMode = VENC_RC_MODE_H264AVBR;
			attr.stAttrH264AVbr.u32Gop = _gop;
			attr.stAttrH264AVbr.u32StatTime = stattime;
			attr.stAttrH264AVbr.u32SrcFrmRate = ifr;
			attr.stAttrH264AVbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH264AVbr.u32MaxBitRate = _bitrate;
			break;
		case FIXQP:
			attr.enRcMode = VENC_RC_MODE_H264FIXQP;
			attr.stAttrH264FixQp.u32Gop = _gop;
			attr.stAttrH264FixQp.u32SrcFrmRate = ifr;
			attr.stAttrH264FixQp.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH264FixQp.u32IQp = 20;
			attr.stAttrH264FixQp.u32PQp = 23;
			break;
		default:
			HIMPP_PRINT("Unsupported RC mode[%d]\n", _rcmode);
			throw IpcamError("Unsupported RC mode");
		}
		break;
	case H265:
		switch (_rcmode) {
		case CBR:
			attr.enRcMode = VENC_RC_MODE_H265CBR;
			attr.stAttrH265Cbr.u32Gop = _gop;
			attr.stAttrH265Cbr.u32StatTime = stattime;
			attr.stAttrH265Cbr.u32SrcFrmRate = ifr;
			attr.stAttrH265Cbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH265Cbr.u32BitRate = _bitrate;
			attr.stAttrH265Cbr.u32FluctuateLevel = 0;
			break;
		case VBR:
			attr.enRcMode = VENC_RC_MODE_H265AVBR;
			attr.stAttrH265AVbr.u32Gop = _gop;
			attr.stAttrH265AVbr.u32StatTime = stattime;
			attr.stAttrH265AVbr.u32SrcFrmRate = ifr;
			attr.stAttrH265AVbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH265AVbr.u32MaxBitRate = _bitrate;
			break;
		case FIXQP:
			attr.enRcMode = VENC_RC_MODE_H265FIXQP;
			attr.stAttrH265FixQp.u32Gop = _gop;
			attr.stAttrH265FixQp.u32SrcFrmRate = ifr;
			attr.stAttrH265FixQp.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrH265FixQp.u32IQp = 20;
			attr.stAttrH265FixQp.u32PQp = 23;
			break;
		default:
			HIMPP_PRINT("Unsupported RC mode[%d]\n", _rcmode);
			throw IpcamError("Unsupported RC mode");
		}
		break;		
	case MJPEG:
		switch (_rcmode) {
		case CBR:
			attr.enRcMode = VENC_RC_MODE_MJPEGCBR;
			attr.stAttrMjpegeCbr.u32StatTime = 1;
			attr.stAttrMjpegeCbr.u32SrcFrmRate = ifr;
			attr.stAttrMjpegeCbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrMjpegeCbr.u32BitRate = _bitrate;
			attr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
			break;
		case VBR:
			attr.enRcMode = VENC_RC_MODE_MJPEGVBR;
			attr.stAttrMjpegeVbr.u32StatTime = 1;
			attr.stAttrMjpegeVbr.u32SrcFrmRate = ifr;
			attr.stAttrMjpegeVbr.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrMjpegeVbr.u32MaxBitRate = _bitrate;
			attr.stAttrMjpegeVbr.u32MinQfactor = 50;
			attr.stAttrMjpegeVbr.u32MaxQfactor = 95;
			break;
		case FIXQP:
			attr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
			attr.stAttrMjpegeFixQp.u32SrcFrmRate = ifr;
			attr.stAttrMjpegeFixQp.fr32DstFrmRate = MIN2(_framerate, ifr);
			attr.stAttrMjpegeFixQp.u32Qfactor = 90;
			break;
		default:
			HIMPP_PRINT("Unsupported RC mode[%d]\n", _rcmode);
			throw IpcamError("Unsupported RC mode");
		}
		break;
	case JPEG:
		break;
	default:
		HIMPP_PRINT("Unsupported encoding code[%d]\n", _encoding);
		throw IpcamError("Unsupported encoding");
	}
}

void HimppVencChan::prepareChnAttr(VENC_CHN_ATTR_S &attr)
{
	HI_U32 stattime;
	stattime = _gop / _framerate;
	stattime = stattime > 0 ? stattime : 1;
	
	switch (_encoding) {
	case H264:
		attr.stVeAttr.enType = PT_H264;
		attr.stVeAttr.stAttrH264e.u32MaxPicWidth = ROUNDUP16(_resolution.width());
		attr.stVeAttr.stAttrH264e.u32MaxPicHeight = ROUNDUP16(_resolution.height());
		attr.stVeAttr.stAttrH264e.u32BufSize = ROUNDUP64(_resolution.width()) * ROUNDUP64(_resolution.height());
		attr.stVeAttr.stAttrH264e.u32PicWidth = _resolution.width();
		attr.stVeAttr.stAttrH264e.u32PicHeight = _resolution.height();
		attr.stVeAttr.stAttrH264e.u32Profile = _h264profile;
		attr.stVeAttr.stAttrH264e.bByFrame = HI_FALSE;
		break;
	case H265:
		attr.stVeAttr.enType = PT_H265;
		attr.stVeAttr.stAttrH265e.u32MaxPicWidth = ROUNDUP16(_resolution.width());
		attr.stVeAttr.stAttrH265e.u32MaxPicHeight = ROUNDUP16(_resolution.height());
		attr.stVeAttr.stAttrH265e.u32BufSize = ROUNDUP64(_resolution.width()) * ROUNDUP64(_resolution.height());
		attr.stVeAttr.stAttrH265e.u32PicWidth = _resolution.width();
		attr.stVeAttr.stAttrH265e.u32PicHeight = _resolution.height();
		attr.stVeAttr.stAttrH265e.u32Profile = _h265profile;
		attr.stVeAttr.stAttrH265e.bByFrame = HI_FALSE;
		break;		
	case MJPEG:
		attr.stVeAttr.enType = PT_MJPEG;
		attr.stVeAttr.stAttrMjpege.u32MaxPicWidth = ROUNDUP16(_resolution.width());
		attr.stVeAttr.stAttrMjpege.u32MaxPicHeight = ROUNDUP16(_resolution.height());
		attr.stVeAttr.stAttrMjpege.u32BufSize = ROUNDUP64(_resolution.width()) * ROUNDUP64(_resolution.height());
		attr.stVeAttr.stAttrMjpege.u32PicWidth = _resolution.width();
		attr.stVeAttr.stAttrMjpege.u32PicHeight = _resolution.height();
		attr.stVeAttr.stAttrMjpege.bByFrame = HI_TRUE;
		break;
	case JPEG:
		attr.stVeAttr.enType = PT_JPEG;
		attr.stVeAttr.stAttrJpege.u32MaxPicWidth = ROUNDUP16(_resolution.width());
		attr.stVeAttr.stAttrJpege.u32MaxPicHeight = ROUNDUP16(_resolution.height());
		attr.stVeAttr.stAttrJpege.u32BufSize = ROUNDUP64(_resolution.width()) * ROUNDUP64(_resolution.height());
		attr.stVeAttr.stAttrJpege.u32PicWidth = _resolution.width();
		attr.stVeAttr.stAttrJpege.u32PicHeight = _resolution.height();
		attr.stVeAttr.stAttrJpege.bByFrame = HI_TRUE;
		attr.stVeAttr.stAttrJpege.bSupportDCF = HI_FALSE;
		break;
	default:
		HIMPP_PRINT("Unsupported encoding code[%d]\n", _encoding);
		throw IpcamError("Unsupported encoding");
	}
	// initialize RateControl attributes
	prepareRcAttr(attr.stRcAttr);
	attr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
	attr.stGopAttr.stNormalP.s32IPQpDelta = 0;
}

void HimppVencChan::doEnableElement()
{
	HI_S32 s32Ret;
	VENC_CROP_CFG_S dis_crop = { .bEnable = HI_FALSE };

	VENC_CHN_ATTR_S attr;
	prepareChnAttr(attr);
	if ((s32Ret = HI_MPI_VENC_CreateChn(_chnid, &attr)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_CreateChn %d faild [%#x]\n",
					_chnid, s32Ret);
		throw IpcamError("Failed to create VENC channel");
	}

	if ((s32Ret = HI_MPI_VENC_SetMaxStreamCnt(_chnid, 2)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_SetMaxStreamCnt %d failed [%#x]\n",
					_chnid, s32Ret);
	}

	if (_encoding == H264) {
		VENC_PARAM_H264_VUI_S stVui;
		if ((s32Ret = HI_MPI_VENC_GetH264Vui(_chnid, &stVui)) == HI_SUCCESS) {
			stVui.stVuiTimeInfo.timing_info_present_flag = 1;
			stVui.stVuiTimeInfo.num_units_in_tick = 1;
			//stVui.stVuiTimeInfo.time_scale = _framerate * 2;
			stVui.stVuiTimeInfo.time_scale = _framerate;
			if ((s32Ret = HI_MPI_VENC_SetH264Vui(_chnid, &stVui)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetH264Vui(%d) failed [%#x]\n",
							_chnid, s32Ret);
			}
		}

		VENC_PARAM_REF_S stRefParam;
		if ((s32Ret = HI_MPI_VENC_GetRefParam(_chnid, &stRefParam)) == HI_SUCCESS) {
			stRefParam.u32Base = _refmode.Base;
			stRefParam.u32Enhance = _refmode.Enhanced;
			stRefParam.bEnablePred = (HI_BOOL)_refmode.EnablePred;
			if ((s32Ret = HI_MPI_VENC_SetRefParam(_chnid, &stRefParam)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetRefParam(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetRefParam(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}

		VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
		if ((s32Ret = HI_MPI_VENC_GetIntraRefresh(_chnid, &stIntraRefresh)) == HI_SUCCESS) {
			stIntraRefresh.bRefreshEnable = (HI_BOOL)_intrarefresh.EnableRefresh;
			stIntraRefresh.bISliceEnable = (HI_BOOL)_intrarefresh.EnableISlice;
			stIntraRefresh.u32RefreshLineNum = _intrarefresh.RefreshLineNum;
			stIntraRefresh.u32ReqIQp = _intrarefresh.ReqIQp;
			if ((s32Ret = HI_MPI_VENC_SetIntraRefresh(_chnid, &stIntraRefresh)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetIntraRefresh(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetIntraRefresh(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}

		VENC_RC_PARAM_S param;
		if ((s32Ret = HI_MPI_VENC_GetRcParam(_chnid, &param)) == HI_SUCCESS) {
			switch (_rcmode) {
			case CBR:
				param.stParamH264Cbr.u32MinQp = _min_qp;
				param.stParamH264Cbr.u32MinIQp = _min_qp;
				param.stParamH264Cbr.u32MaxQp = _max_qp;
				param.stParamH264Cbr.u32MaxIQp = _max_qp;
				break;
			case VBR:
				param.stParamH264AVbr.u32MinQp = _min_qp;
				param.stParamH264AVbr.u32MinIQp = MIN2(_min_qp, param.stParamH264AVbr.u32MaxStillQP);
				param.stParamH264AVbr.u32MaxQp = _max_qp;
				param.stParamH264AVbr.u32MaxIQp = _max_qp;
				break;
			default:
				throw IpcamError("Cannot change MinQP in current rc mode"); break;
			}
			if ((s32Ret = HI_MPI_VENC_SetRcParam(_chnid, &param)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetRcParam(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetRcParam(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}
	}
	if (_encoding == H265) {
		VENC_PARAM_H265_VUI_S stVui;
		if ((s32Ret = HI_MPI_VENC_GetH265Vui(_chnid, &stVui)) == HI_SUCCESS) {
			stVui.stVuiTimeInfo.timing_info_present_flag = 1;
			stVui.stVuiTimeInfo.num_units_in_tick = 1;
			//stVui.stVuiTimeInfo.time_scale = _framerate * 2;
			stVui.stVuiTimeInfo.time_scale = _framerate;
			if ((s32Ret = HI_MPI_VENC_SetH265Vui(_chnid, &stVui)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetH265Vui(%d) failed [%#x]\n",
							_chnid, s32Ret);
			}
		}

		VENC_PARAM_REF_S stRefParam;
		if ((s32Ret = HI_MPI_VENC_GetRefParam(_chnid, &stRefParam)) == HI_SUCCESS) {
			stRefParam.u32Base = _refmode.Base;
			stRefParam.u32Enhance = _refmode.Enhanced;
			stRefParam.bEnablePred = (HI_BOOL)_refmode.EnablePred;
			if ((s32Ret = HI_MPI_VENC_SetRefParam(_chnid, &stRefParam)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetRefParam(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetRefParam(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}

		VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
		if ((s32Ret = HI_MPI_VENC_GetIntraRefresh(_chnid, &stIntraRefresh)) == HI_SUCCESS) {
			stIntraRefresh.bRefreshEnable = (HI_BOOL)_intrarefresh.EnableRefresh;
			stIntraRefresh.bISliceEnable = (HI_BOOL)_intrarefresh.EnableISlice;
			stIntraRefresh.u32RefreshLineNum = _intrarefresh.RefreshLineNum;
			stIntraRefresh.u32ReqIQp = _intrarefresh.ReqIQp;
			if ((s32Ret = HI_MPI_VENC_SetIntraRefresh(_chnid, &stIntraRefresh)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetIntraRefresh(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetIntraRefresh(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}

		VENC_RC_PARAM_S param;
		if ((s32Ret = HI_MPI_VENC_GetRcParam(_chnid, &param)) == HI_SUCCESS) {
			switch (_rcmode) {
			case CBR:
				param.stParamH265Cbr.u32MinQp = _min_qp;
				param.stParamH265Cbr.u32MinIQp = _min_qp;
				param.stParamH265Cbr.u32MaxQp = _max_qp;
				param.stParamH265Cbr.u32MaxIQp = _max_qp;
				break;
			case VBR:
				param.stParamH265AVbr.u32MinQp = _min_qp;
				param.stParamH265AVbr.u32MinIQp = MIN2(_min_qp, param.stParamH265AVbr.u32MaxStillQP);
				param.stParamH265AVbr.u32MaxQp = _max_qp;
				param.stParamH265AVbr.u32MaxIQp = _max_qp;
				break;
			default:
				throw IpcamError("Cannot change MinQP in current rc mode"); break;
			}
			if ((s32Ret = HI_MPI_VENC_SetRcParam(_chnid, &param)) != HI_SUCCESS) {
				HIMPP_PRINT("HI_MPI_VENC_SetRcParam(%d) failed [%#x]\n",
				            _chnid, s32Ret);
			}
		} else {
			HIMPP_PRINT("HI_MPI_VENC_GetRcParam(%d) failed [%#x]\n",
			            _chnid, s32Ret);
		}
	}

	Resolution in = HIMPP_VIDEO_ELEMENT(source())->resolution();
	calc_crop_cfg(in, _resolution, _crop_cfg);
	if ((s32Ret = HI_MPI_VENC_SetCrop(_chnid, &_crop_cfg)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_SetCrop [%d] faild [%#x]!\n",
					 _chnid, s32Ret);
		goto err_destroy_chn;
	}

	if ((s32Ret = HI_MPI_VENC_StartRecvPic(_chnid)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_StartRecvPic %d failed [%#x]\n",
					_chnid, s32Ret);
		goto err_disable_crop;
	}

	MPP_CHN_S dst_chn;
	dst_chn.enModId = HI_ID_VENC;
	dst_chn.s32DevId = 0;
	dst_chn.s32ChnId = _chnid;
	if ((s32Ret = HI_MPI_SYS_Bind(HIMPP_VIDEO_ELEMENT(source())->bindSource(), &dst_chn)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_SYS_Bind %d failed [%#x]\n",
					_chnid, s32Ret);
		goto err_stop_recv_pic;
	}

	for (auto it : _osds) {
		it->enable();
	}

	return;

err_stop_recv_pic:
	HI_MPI_VENC_StopRecvPic(_chnid);
err_disable_crop:
	HI_MPI_VENC_SetCrop(_chnid, &dis_crop);
err_destroy_chn:
	HI_MPI_VENC_DestroyChn(_chnid);

	throw IpcamError("Failed to enable VENC streaming");
}

void HimppVencChan::doDisableElement()
{
	HimppVideoElement* src = HIMPP_VIDEO_ELEMENT(source());
	HI_S32 s32Ret;
	VENC_CROP_CFG_S dis_crop = { .bEnable = HI_FALSE };

	for (auto it : _osds) {
		if (it->is_enabled())
			it->disable();
	}

	MPP_CHN_S dst_chn = {
		.enModId = HI_ID_VENC,
		.s32DevId = 0,
		.s32ChnId = _chnid
	};
	if ((s32Ret = HI_MPI_SYS_UnBind(src->bindSource(), &dst_chn)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_SYS_UnBind %d failed [%#x]\n",
					_chnid, s32Ret);
	}

	if ((s32Ret = HI_MPI_VENC_StopRecvPic(_chnid)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_StopRecvPic %d failed [%#x]\n",
					_chnid, s32Ret);
	}

	if ((s32Ret = HI_MPI_VENC_SetCrop(_chnid, &dis_crop)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_SetCrop [%d] failed [%#x]!\n",
					 _chnid, s32Ret);
	}

	if ((s32Ret = HI_MPI_VENC_DestroyChn(_chnid)) != HI_SUCCESS) {
		HIMPP_PRINT("HI_MPI_VENC_DestroyChn %d failed [%#x]\n",
					_chnid, s32Ret);
	}
}
