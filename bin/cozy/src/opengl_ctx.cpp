#include "opengl_ctx.hpp"

#define SCFLOAT(f) static_cast<float>(f)

namespace rvi
{
    opengl_ctx::opengl_ctx(runtime* rptr, int cid)
        : _runtime_ptr(rptr)
        , _client_id(cid)
    {
        init_default_shaders(&_shader_program);
        glLineWidth(2);
    }

    void opengl_ctx::refresh()
    {
        auto snapshot = _runtime_ptr->snapshot_diff_relative(_client_id);
        for(auto&& frame_entry : snapshot.entries)
        {
            if(frame_entry.deleted)
            {
                _vframes.erase(frame_entry.name);
                continue;
            }

            GLuint vao, vbo;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            vframe vf;
            vf.vao = vao;
            vf.vbo = vbo;
            for(auto& line : frame_entry.lines)
            {
                vf.line_data.push_back(line.start.position.x);
                vf.line_data.push_back(line.start.position.y);
                vf.line_data.push_back(SCFLOAT(line.start.color.rgba()));
                vf.line_data.push_back(line.end.position.x);
                vf.line_data.push_back(line.end.position.y);
                vf.line_data.push_back(SCFLOAT(line.end.color.rgba()));
            }
            _vframes.emplace(frame_entry.name, std::move(vf));

            const vframe& entry = _vframes[frame_entry.name];

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);

            glBufferData(
                GL_ARRAY_BUFFER, 
                entry.line_data.size() * sizeof(float),
                entry.line_data.data(),
                GL_DYNAMIC_DRAW
            );

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, NULL);
            glBindVertexArray(NULL);
        }
    }

    void opengl_ctx::draw(float delta_time)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(_shader_program);
        GLint uloc = glGetUniformLocation(_shader_program, "delta_time");
        if(uloc >= 0)
        {
            glUniform1f(uloc, delta_time);
        }
        
        for(auto &vfp : _vframes)
        {
            glBindVertexArray(vfp.second.vao);
            glDrawArrays(GL_LINES, 0, vfp.second.line_data.size() * sizeof(line));
            glBindVertexArray(NULL);
        }   
    }
}