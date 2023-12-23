// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    template <typename element_type>
    class CommandBuilder
    {
    private:
        std::shared_ptr<String<element_type> > word;
        std::vector<std::shared_ptr<String<element_type> > >* command;

    public:
        CommandBuilder(std::vector<std::shared_ptr<String<element_type> > >* command) :
            command(command),
            word(std::make_shared<String<element_type> >())
        {
        }

        void reset()
        {
            this->word = std::make_shared<String<element_type> >();
        }

        bool write_element(element_type b)
        {
            switch (b)
            {
            case '\r':
                this->command->push_back(this->word);
                return true;

            case ' ':
                this->command->push_back(this->word);
                this->word = std::make_shared<String<element_type> >();
                return false;

            default:
                this->word->push_back(b);
                return false;
            }
        }
    };
}