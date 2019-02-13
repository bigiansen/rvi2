#include <rvi/frame.hpp>

#include <stack>

namespace rvi
{
    frame::frame(const std::string& name, frame* parent)
        : _name(name)
        , _parent(parent)
        , _transform(DEFAULT_TRANSFORM)
    { }

    frame::frame(std::string&& name, frame* parent)
        : _name(std::move(name))
        , _parent(parent)
        , _transform(DEFAULT_TRANSFORM)
    { }

    std::unique_ptr<frame> frame::create_copy(frame* fptr_parent)
    {
        auto result = std::make_unique<frame>(_name, fptr_parent);

        result->_lines = _lines;

        // copy children
        for (auto& chfr_up : _children)
        {
            auto fuptr = std::make_unique<frame>(chfr_up->name(), this);
            result->_children.push_back(std::move(fuptr));
        }

        // regenerate children index
        for (auto& chfr_up : result->_children)
        {
            result->_child_frames_index.emplace(chfr_up->name(), chfr_up.get());
        }

        result->_transform = _transform;
        result->_cached_abs_tform = _cached_abs_tform;
        result->_cached_abs_tform_rebuild = _cached_abs_tform_rebuild;

        return result;
    }

    bool frame::has_parent() const noexcept
    {
        return _parent != nullptr;
    }

    void frame::clear_lines() noexcept
    {
        _lines.clear();
    }

    void frame::add_line(const line& ln)
    {
        _lines.push_back(
            ln.start.position, 
            ln.start.color,
            ln.end.position,
            ln.end.color
        );
    }

    frame* frame::add_child(const std::string& name)
    {
        if (_child_frames_index.count(name) > 0)
        {
            return _child_frames_index.at(name);
        }
        else
        {
            _children.push_back(std::make_unique<frame>(name, this));
            frame* f_ptr = _children.back().get();
            _child_frames_index.emplace(name, f_ptr);
            return f_ptr;
        }
    }

    frame* frame::add_child(std::string&& name)
    {
        if (_child_frames_index.count(name) > 0)
        {
            return _child_frames_index.at(name);
        }
        else
        {
            _children.push_back(std::make_unique<frame>(name, this));
            frame* f_ptr = _children.back().get();
            _child_frames_index.emplace(std::move(name), f_ptr);
            return f_ptr;
        }
    }

    void frame::clear_children()
    {
        _children.clear();
        _child_frames_index.clear();
    }

    bool frame::delete_child(const std::string& name)
    {
        if (_child_frames_index.count(name) == 0)
        {
            return false;
        }
        
        frame* f_ptr = _child_frames_index.at(name);

        auto it = std::find_if(_children.begin(), _children.end(), [&](auto& fuptr)
        {
            return fuptr.get() == f_ptr;
        });

        _children.erase(it);
        _child_frames_index.erase(name);
        
        return true;
    }

    bool frame::contains_child(const std::string& name)
    {
        return (_child_frames_index.count(name) > 0);
    }

    bool frame::contains_child(frame* fptr)
    {
        auto it = std::find_if(_children.begin(), _children.end(), [fptr](auto& ch)
        {
            return ch.get() == fptr;
        });
        return it != _children.end();
    }

    size_t frame::child_count(bool deep) const
    {
        if (!deep)
        {
            return _child_frames_index.size();
        }
        else
        {
            size_t result = 0;
            for (const auto& f : _child_frames_index)
            {
                result++;
                result += _child_frames_index.at(f.first)->child_count(true);
            }
            return result;
        }
    }

    void frame::set_transform(const transform2& tform) noexcept
    {
        _transform = tform;
        _cached_abs_tform_rebuild = true;
    }

    void frame::set_transform(transform2&& tform) noexcept
    {
        _transform = std::move(tform);
        _cached_abs_tform_rebuild = true;
    }

    void frame::set_transform_scale_absolute(bool enabled) noexcept
    {
        _transform_scale_absolute = enabled;
    }

    void frame::set_position(vector2 offset) noexcept
    {
        _transform.position = offset;
        _cached_abs_tform_rebuild = true;
    }

    void frame::set_rotation(float rotation) noexcept
    {
        _transform.rotation = rotation;
        _cached_abs_tform_rebuild = true;
    }

    void frame::set_scale(vector2 scale) noexcept
    {
        _transform.scale = scale;
        _cached_abs_tform_rebuild = true;
    }

    const std::string& frame::name() const noexcept
    {
        return _name;
    }

    line_container& frame::lines()
    {
        return _lines;
    }

    std::unordered_map<std::string, frame*> frame::children() const
    {
        std::unordered_map<std::string, frame*> result;
        result.reserve(_child_frames_index.size());
        for (auto& p : _child_frames_index)
        {
            const std::string& name = p.first;
            frame* f_ptr = _child_frames_index.at(name);
            result.emplace(name, f_ptr);
        }

        return result;
    }

    const transform2& frame::transform() const noexcept
    {
        return _transform;
    }

    bool frame::transform_scale_abs()
    {
        return _transform_scale_absolute;
    }

    frame* frame::parent() const noexcept
    {
        return _parent;
    }

    transform2 frame::get_absolute_transform()
    {
        if(_cached_abs_tform_rebuild)
        {
            std::stack<transform2> tform_stack;
            tform_stack.push(_transform);

            const frame* current = this;
            while(current->has_parent())
            {
                current = current->_parent;
                tform_stack.push(current->_transform);
            }

            _cached_abs_tform = transform2::default_value();
            while(!tform_stack.empty())
            {
                const transform2 tf = tform_stack.top();
                _cached_abs_tform.merge_in_place(tf);
                tform_stack.pop();
            }
            _cached_abs_tform_rebuild = false;
        }

        return _cached_abs_tform;
    }

    rectangle frame::bounding_rect() const noexcept
    {
        return rectangle(_transform.position, _transform.scale);
    }

    frame* frame::get_child(const std::string& name)
    {
        if(_child_frames_index.count(name) == 0)
        {
            return nullptr;
        }
        return _child_frames_index.at(name);
    }

    size_t frame::line_count() const noexcept
    {
        return _lines.size();
    }

    void frame::distort(vector2 ul, vector2 ur, vector2 ll, vector2 lr)
    {
        auto distort = [ul, ur, ll, lr](frame* fptr)
        {
            auto& lines = fptr->lines();
            for(auto it = lines.position_begin(); it != lines.position_end(); it += 2)
            {
                float& x = *it;
                float& y = *(it + 1);

                float offset_x = 0.0F;
                float offset_y = 0.0F;

                // ul
                float ul_f_sample = y * (1 - x);
                offset_x += ul.x * ul_f_sample;
                offset_y += ul.y * ul_f_sample;

                // ur
                float ur_f_sample = y * x;
                offset_x += ur.x * ur_f_sample;
                offset_y += ur.y * ur_f_sample;

                // ll
                float ll_f_sample = (1 - y) * (1 - x);
                offset_x += ll.x * ll_f_sample;
                offset_y += ll.y * ll_f_sample;

                // lr
                float lr_f_sample = (1 - y) * x;
                offset_x += lr.x * lr_f_sample;
                offset_y += lr.y * lr_f_sample;

                x += offset_x;
                y += offset_y;
            }
        };
        distort(this);
        for(auto& ch_uptr : _children)
        {
            distort(ch_uptr.get());
        }
    }
}