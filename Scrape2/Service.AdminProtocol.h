// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.CommandBuilder.h"
#include "Basic.StateMachine.h"
#include "Basic.ITransportEventHandler.h"
#include "Basic.IStream.h"
#include "Basic.NameValueCollection.h"
#include "Web.Client.h"
#include "Html.Document.h"
#include "Html.Types.h"
#include "Html.Parser.h"
#include "Basic.Lock.h"
#include "Web.Page.h"
#include "Web.Form.h"
#include "Basic.IStream.h"
#include "Scrape.Netflix.h"
#include "Scrape.Amazon.h"

namespace Service
{
    using namespace Basic;

    class AdminProtocol : public StateMachine, public ITransportEventHandler<Codepoint>, public std::enable_shared_from_this<AdminProtocol>
    {
    private:
        enum State
        {
            start_state = Start_State,
            command_frame_pending_state,
            done_state = Succeeded_State,
            peer_write_failed,
            peer_flush_failed,
        };

        std::vector<UnicodeStringRef> command;
        Basic::CommandBuilder<Codepoint> command_frame;
        std::shared_ptr<IStream<Codepoint> > transport;
        std::shared_ptr<Web::Client> client;
        std::shared_ptr<Html::Parser> html_parser;
        std::shared_ptr<Web::Page> current_page;
        std::shared_ptr<Web::Form> current_form;
        Basic::ByteStringRef get_cookie;

		ByteStringRef amazon_cookie;
		ByteStringRef netflix_cookie;
		std::shared_ptr<Scrape::Amazon> amazon_scrape;
		std::shared_ptr<Scrape::Netflix> netflix_scrape;

        void write_to_human_with_context(Html::Node* node, Basic::IStream<Codepoint>* stream, bool verbose);
        void consider_event();

    public:
        AdminProtocol(std::shared_ptr<Basic::IStream<Codepoint> > transport);

        virtual void ITransportEventHandler<Codepoint>::transport_connected();
        virtual void ITransportEventHandler<Codepoint>::transport_disconnected();
        virtual void ITransportEventHandler<Codepoint>::transport_received(const Codepoint* elements, uint32 count);
        bool receiver_received(ElementSource<Codepoint>* elements);
        void tbd_event(); // $$$
    };
}