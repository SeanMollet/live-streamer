/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * video-encoder.h
 * Copyright (C) 2017 Watson Xu <xuhuashan@gmail.com>
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

#ifndef _VIDEO_ENCODER_H_
#define _VIDEO_ENCODER_H_

#include <media-common.h>
#include <media-element.h>
#include <video-osd.h>
#include <media-stream.h>

namespace Ipcam {
namespace Media {

enum RateCtrlMode { CBR, VBR, FIXQP };
enum H264Profile { BASELINE, MAIN, HIGH, SVC_T };
enum H265Profile { HEVC_MAIN };

class VideoEncoder;
class H264VideoEncoder;
class H265VideoEncoder;
class JPEGVideoEncoder;

#define VIDEO_ENCODER(o)		dynamic_cast<Ipcam::Media::VideoEncoder*>(o)
#define H264_VIDEO_ENCODER(o)	dynamic_cast<Ipcam::Media::H264VideoEncoder*>(o)
#define H265_VIDEO_ENCODER(o)	dynamic_cast<Ipcam::Media::H265VideoEncoder*>(o)
#define JPEG_VIDEO_ENCODER(o)	dynamic_cast<Ipcam::Media::JPEGVideoEncoder*>(o)

// control interface for video encoder
class VideoEncoder
{
public:
	virtual ~VideoEncoder();

	virtual VideoEncodingType	getEncoding();
	virtual Resolution			getResolution();
	virtual void				setResolution(Resolution value);
	virtual uint32_t			getFramerate();
	virtual void				setFramerate(uint32_t value);
	virtual RateCtrlMode		getRcMode();
	virtual void				setRcMode(RateCtrlMode value);
	virtual uint32_t			getBitrate();
	virtual void				setBitrate(uint32_t value);

	virtual VideoOSD*			CreateOSD();
	virtual void				DeleteOSD(VideoOSD* osd);

protected:
	VideoEncoder();
};

struct FrameRefMode {
	uint32_t				Base;
	uint32_t				Enhanced;
	bool					EnablePred;
	FrameRefMode(uint32_t base, uint32_t enhanced, bool enablepred)
		: Base(base), Enhanced(enhanced), EnablePred(enablepred) {}
};

struct IntraRefreshParam {
	bool					EnableRefresh;
	bool					EnableISlice;
	uint32_t				RefreshLineNum;
	uint32_t				ReqIQp;
	IntraRefreshParam(bool refresh_en, bool islice_en, uint32_t linenum, uint32_t reqiqp)
		: EnableRefresh(refresh_en), EnableISlice(islice_en), RefreshLineNum(linenum), ReqIQp(reqiqp) {}
};

class H264VideoEncoder : public virtual VideoEncoder
{
public:
	virtual ~H264VideoEncoder();

	virtual H264Profile			getH264Profile();
	virtual void				setH264Profile(H264Profile value);
	virtual uint32_t			getGovLength();
	virtual void				setGovLength(uint32_t value);
	virtual uint32_t			getMinQP();
	virtual void				setMinQP(uint32_t value);
	virtual uint32_t			getMaxQP();
	virtual void				setMaxQP(uint32_t value);

	virtual void				setFrameRefMode(FrameRefMode value);
	virtual FrameRefMode		getFrameRefMode();
	virtual void				setIntraRefresh(IntraRefreshParam value);
	virtual IntraRefreshParam	getIntraRefresh();

protected:
	H264VideoEncoder();
};

class H265VideoEncoder : public virtual VideoEncoder
{
public:
	virtual ~H265VideoEncoder();

	virtual H265Profile			getH265Profile();
	virtual void				setH265Profile(H265Profile value);
	virtual uint32_t			getGovLength();
	virtual void				setGovLength(uint32_t value);
	virtual uint32_t			getMinQP();
	virtual void				setMinQP(uint32_t value);
	virtual uint32_t			getMaxQP();
	virtual void				setMaxQP(uint32_t value);

	virtual void				setFrameRefMode(FrameRefMode value);
	virtual FrameRefMode		getFrameRefMode();
	virtual void				setIntraRefresh(IntraRefreshParam value);
	virtual IntraRefreshParam	getIntraRefresh();

protected:
	H265VideoEncoder();
};

class JPEGVideoEncoder : virtual public VideoEncoder
{
public:
	virtual ~JPEGVideoEncoder();

protected:
	JPEGVideoEncoder();
};

} // namespace Media
} // namespace Ipcam

#endif // _VIDEO_ENCODER_H_

