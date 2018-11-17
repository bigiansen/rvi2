#pragma once

#include <client_context.hpp>

#include "client_instance_data.hpp"

namespace rvi
{
    class client_instance
    {
    public:
        client_context context;
        client_instance_data data;

        // -- include control --
        void push_include(const std::string& inc_file);
        const std::string& peek_current_include() const;
        void pop_include();
        void mark_include_once();
    };
}