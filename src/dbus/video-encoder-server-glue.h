
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__video_encoder_server_glue_h__ADAPTOR_MARSHAL_H
#define __dbusxx__video_encoder_server_glue_h__ADAPTOR_MARSHAL_H

#include <dbus-c++/dbus.h>
#include <cassert>

namespace ipcam {
namespace Media {

class VideoEncoder_adaptor
: public ::DBus::InterfaceAdaptor
{
public:

    VideoEncoder_adaptor()
    : ::DBus::InterfaceAdaptor("ipcam.Media.VideoEncoder")
    {
        bind_property(Encoding, "u", true, false);
        bind_property(Resolution, "s", true, true);
    }

    ::DBus::IntrospectedInterface *introspect() const 
    {
        static ::DBus::IntrospectedMethod VideoEncoder_adaptor_methods[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod VideoEncoder_adaptor_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty VideoEncoder_adaptor_properties[] = 
        {
            { "Encoding", "u", true, false },
            { "Resolution", "s", true, true },
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface VideoEncoder_adaptor_interface = 
        {
            "ipcam.Media.VideoEncoder",
            VideoEncoder_adaptor_methods,
            VideoEncoder_adaptor_signals,
            VideoEncoder_adaptor_properties
        };
        return &VideoEncoder_adaptor_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */
    ::DBus::PropertyAdaptor< uint32_t > Encoding;
    ::DBus::PropertyAdaptor< std::string > Resolution;

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
};

} } 
namespace ipcam {
namespace Media {
namespace VideoEncoder {

class RateControl_adaptor
: public ::DBus::InterfaceAdaptor
{
public:

    RateControl_adaptor()
    : ::DBus::InterfaceAdaptor("ipcam.Media.VideoEncoder.RateControl")
    {
        bind_property(RateControlMode, "u", true, true);
        bind_property(FrameRate, "u", true, true);
        bind_property(EncodingInterval, "u", true, true);
        bind_property(Bitrate, "u", true, true);
    }

    ::DBus::IntrospectedInterface *introspect() const 
    {
        static ::DBus::IntrospectedMethod RateControl_adaptor_methods[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod RateControl_adaptor_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty RateControl_adaptor_properties[] = 
        {
            { "RateControlMode", "u", true, true },
            { "FrameRate", "u", true, true },
            { "EncodingInterval", "u", true, true },
            { "Bitrate", "u", true, true },
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface RateControl_adaptor_interface = 
        {
            "ipcam.Media.VideoEncoder.RateControl",
            RateControl_adaptor_methods,
            RateControl_adaptor_signals,
            RateControl_adaptor_properties
        };
        return &RateControl_adaptor_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */
    ::DBus::PropertyAdaptor< uint32_t > RateControlMode;
    ::DBus::PropertyAdaptor< uint32_t > FrameRate;
    ::DBus::PropertyAdaptor< uint32_t > EncodingInterval;
    ::DBus::PropertyAdaptor< uint32_t > Bitrate;

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
};

} } } 
namespace ipcam {
namespace Media {
namespace VideoEncoder {

class H264_adaptor
: public ::DBus::InterfaceAdaptor
{
public:

    H264_adaptor()
    : ::DBus::InterfaceAdaptor("ipcam.Media.VideoEncoder.H264")
    {
        bind_property(H264Profile, "u", true, true);
        bind_property(GovLength, "u", true, true);
        bind_property(MinQP, "u", true, true);
        bind_property(MaxQP, "u", true, true);
        bind_property(FrameRefMode, "(uub)", true, true);
        bind_property(IntraRefresh, "(bbuu)", true, true);
    }

    ::DBus::IntrospectedInterface *introspect() const 
    {
        static ::DBus::IntrospectedMethod H264_adaptor_methods[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod H264_adaptor_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty H264_adaptor_properties[] = 
        {
            { "H264Profile", "u", true, true },
            { "GovLength", "u", true, true },
            { "MinQP", "u", true, true },
            { "MaxQP", "u", true, true },
            { "FrameRefMode", "(uub)", true, true },
            { "IntraRefresh", "(bbuu)", true, true },
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface H264_adaptor_interface = 
        {
            "ipcam.Media.VideoEncoder.H264",
            H264_adaptor_methods,
            H264_adaptor_signals,
            H264_adaptor_properties
        };
        return &H264_adaptor_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */
    ::DBus::PropertyAdaptor< uint32_t > H264Profile;
    ::DBus::PropertyAdaptor< uint32_t > GovLength;
    ::DBus::PropertyAdaptor< uint32_t > MinQP;
    ::DBus::PropertyAdaptor< uint32_t > MaxQP;
    ::DBus::PropertyAdaptor< ::DBus::Struct< uint32_t, uint32_t, bool > > FrameRefMode;
    ::DBus::PropertyAdaptor< ::DBus::Struct< bool, bool, uint32_t, uint32_t > > IntraRefresh;

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
};

} } } 
namespace ipcam {
namespace Media {
namespace VideoEncoder {

class OSD_adaptor
: public ::DBus::InterfaceAdaptor
{
public:

    OSD_adaptor()
    : ::DBus::InterfaceAdaptor("ipcam.Media.VideoEncoder.OSD")
    {
        register_method(OSD_adaptor, CreateOSD, _CreateOSD_stub);
        register_method(OSD_adaptor, DeleteOSD, _DeleteOSD_stub);
        register_method(OSD_adaptor, GetOSDs, _GetOSDs_stub);
    }

    ::DBus::IntrospectedInterface *introspect() const 
    {
        static ::DBus::IntrospectedArgument CreateOSD_args[] = 
        {
            { "index", "u", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument DeleteOSD_args[] = 
        {
            { "index", "u", true },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedArgument GetOSDs_args[] = 
        {
            { "osds", "a{uo}", false },
            { 0, 0, 0 }
        };
        static ::DBus::IntrospectedMethod OSD_adaptor_methods[] = 
        {
            { "CreateOSD", CreateOSD_args },
            { "DeleteOSD", DeleteOSD_args },
            { "GetOSDs", GetOSDs_args },
            { 0, 0 }
        };
        static ::DBus::IntrospectedMethod OSD_adaptor_signals[] = 
        {
            { 0, 0 }
        };
        static ::DBus::IntrospectedProperty OSD_adaptor_properties[] = 
        {
            { 0, 0, 0, 0 }
        };
        static ::DBus::IntrospectedInterface OSD_adaptor_interface = 
        {
            "ipcam.Media.VideoEncoder.OSD",
            OSD_adaptor_methods,
            OSD_adaptor_signals,
            OSD_adaptor_properties
        };
        return &OSD_adaptor_interface;
    }

public:

    /* properties exposed by this interface, use
     * property() and property(value) to get and set a particular property
     */

public:

    /* methods exported by this interface,
     * you will have to implement them in your ObjectAdaptor
     */
    virtual uint32_t CreateOSD() = 0;
    virtual void DeleteOSD(const uint32_t& index) = 0;
    virtual std::map< uint32_t, ::DBus::Path > GetOSDs() = 0;

public:

    /* signal emitters for this interface
     */

private:

    /* unmarshalers (to unpack the DBus message before calling the actual interface method)
     */
    ::DBus::Message _CreateOSD_stub(const ::DBus::CallMessage &call)
    {
        ::DBus::MessageIter ri = call.reader();

        uint32_t argout1 = CreateOSD();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
    ::DBus::Message _DeleteOSD_stub(const ::DBus::CallMessage &call)
    {
        ::DBus::MessageIter ri = call.reader();

        uint32_t argin1; ri >> argin1;
        DeleteOSD(argin1);
        ::DBus::ReturnMessage reply(call);
        return reply;
    }
    ::DBus::Message _GetOSDs_stub(const ::DBus::CallMessage &call)
    {
        ::DBus::MessageIter ri = call.reader();

        std::map< uint32_t, ::DBus::Path > argout1 = GetOSDs();
        ::DBus::ReturnMessage reply(call);
        ::DBus::MessageIter wi = reply.writer();
        wi << argout1;
        return reply;
    }
};

} } } 
#endif //__dbusxx__video_encoder_server_glue_h__ADAPTOR_MARSHAL_H