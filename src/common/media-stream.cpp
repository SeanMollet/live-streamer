/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * media-stream.cpp
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

#include "media-stream.h"

namespace Ipcam {
namespace Media {


//////////////////////////////////////////////////////////////////////////////
// StreamSource
//////////////////////////////////////////////////////////////////////////////

StreamSource::~StreamSource()
{
}

void StreamSource::attach(StreamSink* sink)
{
	if (!dynamic_cast<StreamSink*>(sink)) return; // Sanity check

	_sinks.insert(sink);
}

void StreamSource::detach(StreamSink* sink)
{
	if (!dynamic_cast<StreamSink*>(sink)) return; // Sanity check

	_sinks.erase(sink);
}

void StreamSource::streamData(StreamBuffer* buffer)
{
	for (auto sink : _sinks) {
		sink->streamData(buffer);
	}
}


//////////////////////////////////////////////////////////////////////////////
// StreamSink
//////////////////////////////////////////////////////////////////////////////

void StreamSink::play()
{
}

void StreamSink::stop()
{
}

void StreamSink::pause()
{
}

void StreamSink::resume()
{
}


//////////////////////////////////////////////////////////////////////////////
// VideoStreamSource
//////////////////////////////////////////////////////////////////////////////

VideoStreamSource::~VideoStreamSource()
{
}

//////////////////////////////////////////////////////////////////////////////
// H264VideoStreamSource
//////////////////////////////////////////////////////////////////////////////

H264VideoStreamSource::~H264VideoStreamSource()
{
}

//////////////////////////////////////////////////////////////////////////////
// H265VideoStreamSource
//////////////////////////////////////////////////////////////////////////////

H265VideoStreamSource::~H265VideoStreamSource()
{
}


//////////////////////////////////////////////////////////////////////////////
// JPEGVideoStreamSource
//////////////////////////////////////////////////////////////////////////////

JPEGVideoStreamSource::~JPEGVideoStreamSource()
{
}

//////////////////////////////////////////////////////////////////////////////
// AudioStreamSource
//////////////////////////////////////////////////////////////////////////////

AudioStreamSource::~AudioStreamSource()
{
}

} //namespace Media
} //namespace Ipcam
