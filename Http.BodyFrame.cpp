// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.IgnoreFrame.h"
#include "Basic.ProcessStream.h"
#include "Gzip.FileFormat.h"
#include "Http.BodyFrame.h"
#include "Http.Globals.h"
#include "Http.LengthBodyFrame.h"
#include "Http.BodyChunksFrame.h"
#include "Http.DisconnectBodyFrame.h"

namespace Http
{
    using namespace Basic;

    BodyFrame::BodyFrame(std::shared_ptr<IStream<byte> > decoded_content_stream) :
        decoded_content_stream(decoded_content_stream)
    {
    }
    
    void BodyFrame::make_body_frame(std::shared_ptr<IStream<byte> > decoded_content_stream, Transaction* transaction, std::shared_ptr<BodyFrame>* output)
    {
        uint16 code = transaction->response->code;

        bool success;
        bool body_expected = !(code / 100 == 1 || code == 204 || code == 205 || code == 304 || equals<UnicodeString, true>(transaction->request->method.get(), Http::globals->head_method.get()));

        if (body_expected)
        {
            // get the body frame set up first, because the ResponseHeadersEvent completion can recurse into
            // this class to set the decoded content stream on the body frame

            UnicodeStringRef contentType;
            success = transaction->response->headers->get_string(Http::globals->header_content_type, &contentType);
            if (!success)
            {
                (*output) = std::shared_ptr<BodyFrame>();
                return;
            }
        }

        UnicodeStringRef contentEncoding;
        success = transaction->response->headers->get_string(Http::globals->header_content_encoding, &contentEncoding);
        if (success)
        {
            if (equals<UnicodeString, false>(contentEncoding.get(), Http::globals->gzip.get()))
            {
                auto content_encoder_frame = std::make_shared<Gzip::FileFormat>(decoded_content_stream);
                decoded_content_stream = std::make_shared<ProcessStream<byte> >(content_encoder_frame);
            }
            else if (!equals<UnicodeString, false>(contentEncoding.get(), Http::globals->identity.get()))
            {
                ByteString error;
                error.append((byte*)"unhandled content encoding=");
                ascii_encode(contentEncoding.get(), &error);

                HandleError((char*)error.c_str());

                (*output) = std::shared_ptr<BodyFrame>();
                return;
            }
        }

        if (!decoded_content_stream)
            decoded_content_stream = std::make_shared<IgnoreFrame<byte> >();

        UnicodeStringRef transferEncoding;
        success = transaction->response->headers->get_string(Http::globals->header_transfer_encoding, &transferEncoding);
        if (success)
        {
            if (equals<UnicodeString, false>(transferEncoding.get(), Http::globals->chunked.get()))
            {
                (*output) = std::make_shared<BodyChunksFrame>(decoded_content_stream, transaction->response->headers);
                return;
            }

            if (!equals<UnicodeString, false>(transferEncoding.get(), Http::globals->identity.get()))
            {
                (*output) = std::shared_ptr<BodyFrame>();
                return;
            }
        }

        // $ why does transfer-length take precedence over content-length? if we have both what should we do?
        uint32 contentLength;
        success = transaction->response->headers->get_base_10(Http::globals->header_transfer_length, &contentLength);
        if (success)
        {
            if (contentLength == 0)
            {
                (*output) = std::shared_ptr<BodyFrame>();
                return;
            }

            (*output) = std::make_shared<LengthBodyFrame>(decoded_content_stream, contentLength);
            return;
        }

        success = transaction->response->headers->get_base_10(Http::globals->header_content_length, &contentLength);
        if (success)
        {
            if (contentLength == 0)
            {
                (*output) = std::shared_ptr<BodyFrame>();
                return;
            }
            
            (*output) = std::make_shared<LengthBodyFrame>(decoded_content_stream, contentLength);
            return;
        }

        // if there is no content length, then the body is terminated by transport disconnect
        (*output) = std::make_shared<DisconnectBodyFrame>(decoded_content_stream);
    }
}