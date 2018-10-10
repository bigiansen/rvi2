#include "serialization_base.hpp"

namespace rvi::serialization
{
    void serialize_fp32_bf(data_t& buff, float val)
    {
        static_assert(0, "NI");
    }

    void serialize_fp64_bf(data_t& buff, double val)
    {
        static_assert(0, "NI");
    }

    void serialize_line_bf(data_t& buff, const rvi::line& val)
    {
        serialize_vertex_bf(buff, val.start);
        serialize_vertex_bf(buff, val.end);
    }

    void serialize_vertex_bf(data_t& buff, const vertex& val)
    {
        serialize_vector2_bf(buff, val.position);
        serialize_color_rgba_bf(buff, val.color);
    }

    void serialize_vector2_bf(data_t& buff, const vector2& val)
    {
        serialize_fp32_bf(buff, val.x);
        serialize_fp32_bf(buff, val.y);
    }

    void serialize_color_rgba_bf(data_t& buff, const color_rgba& val)
    {
        buff.push_back(val.r);
        buff.push_back(val.g);
        buff.push_back(val.b);
        buff.push_back(val.a);
    }

    void serialize_transform2_bf(data_t& buff, const rvi::transform2& val)
    {
        serialize_vector2_bf(buff, val.position);
        serialize_vector2_bf(buff, val.scale);
        serialize_fp32_bf(buff, val.rotation);
    }

    cmd_header get_cmd_header(data_t& buff, size_t offset)
    {
        uint8_t h_val = buff[offset + 0];
        return static_cast<cmd_header>(h_val);
    }

    float deserialize_fp32(const data_t& buff, size_t offset)
    {
        static_assert(0, "NI");
    }

    double deserialize_fp64(const data_t& buff, size_t offset)
    {
        static_assert(0, "NI");
    }

    vector2 deserialize_vector2(const data_t& buff, size_t offset)
    {
        float x = deserialize_fp32(buff, offset);
        float y = deserialize_fp32(buff, offset + sizeof(float));
        return vector2(x, y);
    }

    color_rgba deserialize_color_rgba(const data_t& buff, size_t offset)
    {
        uint8_t r = buff[offset + 0];
        uint8_t g = buff[offset + 1];
        uint8_t b = buff[offset + 2];
        uint8_t a = buff[offset + 3];
        return color_rgba(r, g, b, a);
    }

    vertex deserialize_vertex(const data_t& buff, size_t offset)
    {
        vector2 pos = deserialize_vector2(buff, offset);
        color_rgba col = deserialize_color_rgba(buff, sizeof(vector2));
        return vertex(pos, col);
    }

    transform2 deserialize_transform2(const data_t& buff, size_t offset)
    {
        transform2 result = transform2::default_value();
        result.position = deserialize_vector2(buff, offset);
        result.scale = deserialize_vector2(buff, offset + sizeof(vector2));
        result.rotation = deserialize_fp32(buff, offset + (sizeof(vector2) * 2));
        return result;
    }

    line deserialize_line(const data_t& buff, size_t offset)
    {
        line result;
        result.start = deserialize_vertex(buff, offset);
        result.end = deserialize_vertex(buff, offset + sizeof(vertex));
        return result;
    }
}