#ifndef GPUR_WINDOW_HPP
# define GPUR_WINDOW_HPP

class GpuWindow
{
    public:
        GpuWindow();
        GpuWindow(const GpuWindow &other);
        virtual ~GpuWindow();
        GpuWindow &operator=(const GpuWindow &other);


        virtual bool initialize(const char *title, int width, int height,
            bool fullscreen) noexcept = 0;
        virtual void destroy() noexcept = 0;
        virtual void poll_events() noexcept = 0;
        virtual void swap_buffers() noexcept = 0;
        virtual bool should_close() const noexcept = 0;
        virtual int  get_width()  const noexcept = 0;
        virtual int  get_height() const noexcept = 0;

        virtual int  get_mouse_x() const noexcept = 0;
        virtual int  get_mouse_y() const noexcept = 0;
        virtual bool was_mouse_clicked() const noexcept = 0;
        virtual void set_cursor_visible(bool visible) noexcept = 0;
        virtual bool was_settings_key_pressed() const noexcept = 0;

        static GpuWindow *create() noexcept;
};

#endif
