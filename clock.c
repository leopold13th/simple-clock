/*
Desktop Clock Application
Uses Xlib + Cairo for rendering
Compile: gcc -o clock clock.c $(pkg-config --cflags --libs cairo x11) -lm
*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#define WINDOW_TITLE "Desktop Clock"
/*
#define WINDOW_SIZE 400
#define FONT_SIZE 24
#define NUMBERS_SHIFT 55
#define HOUR_MARKERS_SHIFT1 15
#define HOUR_MARKERS_SHIFT2 20
#define MINUTE_MARKERS_SHIFT1 15
#define MINUTE_MARKERS_SHIFT2 22
#define HOUR_HAND_SIZE 100
#define MINUTE_HAND_SIZE 60

#define WINDOW_SIZE 250
#define FONT_SIZE 20
#define NUMBERS_SHIFT 30
#define HOUR_MARKERS_SHIFT1 5
#define HOUR_MARKERS_SHIFT2 12
#define MINUTE_MARKERS_SHIFT1 5
#define MINUTE_MARKERS_SHIFT2 10
#define HOUR_HAND_SIZE 60
#define MINUTE_HAND_SIZE 30
*/

#define WINDOW_SIZE 220
#define BORDER_SIZE 5
#define FONT_SIZE 18
#define NUMBERS_SHIFT 27
#define HOUR_MARKERS_SHIFT1 5
#define HOUR_MARKERS_SHIFT2 12
#define MINUTE_MARKERS_SHIFT1 5
#define MINUTE_MARKERS_SHIFT2 10
#define HOUR_HAND_SIZE 60
#define MINUTE_HAND_SIZE 30


typedef struct {
    Display *display;
    Window window;
    // Поверхности и контексты для двойной буферизации
    cairo_surface_t *xlib_surface;
    cairo_t *xlib_cr;
    cairo_surface_t *image_surface;
    cairo_t *image_cr;
    int width;
    int height;
    gboolean show_digital;
    gboolean show_seconds;
} ClockApp;

void draw_clock(ClockApp *app) {
    // Рисуем в off-screen буфер, а не сразу на экран
    cairo_t *cr = app->image_cr;
    time_t now;
    struct tm *tm_info;
    double center_x = app->width / 2.0;
    double center_y = app->height / 2.0;
    double radius = app->width / 2.0 - BORDER_SIZE;

    // Get current time
    time(&now);
    tm_info = localtime(&now);

    // Clear background with dark theme
    cairo_set_source_rgb(cr, 0, 0, 0);
    // cairo_set_source_rgb(cr, 0.15, 0.15, 0.2);
    // cairo_set_source_rgb(cr, 149/255.0, 158/255.0, 128/255.0);
    // cairo_set_source_rgb(cr, 104/255.0, 120/255.0, 136/255.0);
    cairo_paint(cr);

    // Draw outer clock circle
    cairo_move_to(cr, center_x+radius, center_y);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, center_x, center_y, radius, 0, 2 * M_PI);
    cairo_fill(cr);

/*
    // Draw inner circle
    cairo_set_line_width(cr, 8);
    cairo_set_source_rgb(cr, 0.25, 0.25, 0.35);
    cairo_arc(cr, center_x, center_y, radius + 15, 0, 2 * M_PI);
    cairo_stroke(cr);
*/

    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        double angle = (i * M_PI / 6.0) - (M_PI / 2.0);
        double x1 = center_x + (radius - HOUR_MARKERS_SHIFT1) * cos(angle);
        double y1 = center_y + (radius - HOUR_MARKERS_SHIFT1) * sin(angle);
        double x2 = center_x + (radius - HOUR_MARKERS_SHIFT2) * cos(angle);
        double y2 = center_y + (radius - HOUR_MARKERS_SHIFT2) * sin(angle);

        // cairo_set_line_width(cr, i % 3 == 0 ? 4 : 2);
        cairo_set_line_width(cr, 3);
        cairo_set_source_rgb(cr, 0.15, 0.15, 0.2);
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    // Draw minute markers
    for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        double angle = (i * M_PI / 30.0) - (M_PI / 2.0);
        double x1 = center_x + (radius - MINUTE_MARKERS_SHIFT1) * cos(angle);
        double y1 = center_y + (radius - MINUTE_MARKERS_SHIFT1) * sin(angle);
        double x2 = center_x + (radius - MINUTE_MARKERS_SHIFT2) * cos(angle);
        double y2 = center_y + (radius - MINUTE_MARKERS_SHIFT2) * sin(angle);

        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0.1, 0.1, 0.3);
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
    }

    // Draw hour numbers
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, FONT_SIZE);
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.25);

    for (int i = 1; i <= 12; i++) {
        double angle = (i * M_PI / 6.0) - (M_PI / 2.0);
        double x = center_x + (radius - NUMBERS_SHIFT) * cos(angle);
        double y = center_y + (radius - NUMBERS_SHIFT) * sin(angle);

        cairo_text_extents_t extents;
        char num_str[3];
        snprintf(num_str, sizeof(num_str), "%d", i);
        cairo_text_extents(cr, num_str, &extents);

        // cairo_save(cr);
        cairo_move_to(cr, x - extents.width / 2 - extents.x_bearing,
                          y - extents.height / 2 - extents.y_bearing);
        cairo_show_text(cr, num_str);
        // cairo_restore(cr);
    }

    // Calculate hand angles
    int hour = tm_info->tm_hour % 12;
    int minute = tm_info->tm_min;
    int second = tm_info->tm_sec;

    double hour_angle = ((hour + minute / 60.0) * M_PI / 6.0) - (M_PI / 2.0);
    double minute_angle = ((minute + second / 60.0) * M_PI / 30.0) - (M_PI / 2.0);
    double second_angle = (second * M_PI / 30.0) - (M_PI / 2.0);

    // Draw hour hand
    cairo_set_line_width(cr, 6);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_source_rgb(cr, 0.25, 0.25, 0.28);
    cairo_move_to(cr,
                  center_x - 15 * cos(hour_angle),
                  center_y - 15 * sin(hour_angle));
    cairo_line_to(cr,
                  center_x + (radius - HOUR_HAND_SIZE) * cos(hour_angle),
                  center_y + (radius - HOUR_HAND_SIZE) * sin(hour_angle));
    cairo_stroke(cr);

    // Draw minute hand
    cairo_set_line_width(cr, 5);
    cairo_set_source_rgb(cr, 0.25, 0.25, 0.25);
    cairo_move_to(cr,
                  center_x - 20 * cos(minute_angle),
                  center_y - 20 * sin(minute_angle));
    cairo_line_to(cr,
                  center_x + (radius - MINUTE_HAND_SIZE) * cos(minute_angle),
                  center_y + (radius - MINUTE_HAND_SIZE) * sin(minute_angle));
    cairo_stroke(cr);

    // Draw second hand
    if (app->show_seconds) {
        cairo_set_line_width(cr, 2);
        cairo_set_source_rgb(cr, 0.95, 0.3, 0.3);
        cairo_move_to(cr,
                      center_x - 20 * cos(second_angle),
                      center_y - 20 * sin(second_angle));
        cairo_line_to(cr,
                      center_x + (radius - 5) * cos(second_angle),
                      center_y + (radius - 5) * sin(second_angle));
        cairo_stroke(cr);

        // Second hand center circle
        cairo_set_source_rgb(cr, 0.95, 0.3, 0.3);
        cairo_arc(cr, center_x, center_y, 7, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    // Флешим буфер, чтобы гарантировать завершение всех операций рисования
    cairo_surface_flush(app->image_surface);
}

int main(int argc, char *argv[]) {
    ClockApp app;
    memset(&app, 0, sizeof(ClockApp));

    app.width  = WINDOW_SIZE;
    app.height = WINDOW_SIZE;
    app.show_digital = 1;
    app.show_seconds = 1;

    // Open X display
    app.display = XOpenDisplay(NULL);
    if (!app.display) {
        fprintf(stderr, "Error: Cannot open X display\n");
        return 1;
    }

    int screen = DefaultScreen(app.display);
    Window root = RootWindow(app.display, screen);

    // Create window
    app.window = XCreateSimpleWindow(app.display, root,
                                     100, 100,
                                     app.width, app.height,
                                     0,
                                     BlackPixel(app.display, screen),
                                     BlackPixel(app.display, screen));

    // Важно: задаем фон "None", чтобы окно не очищалось само (иначе будет мерцание)
    XSetWindowBackgroundPixmap(app.display, app.window, None);

    // Set window title
    XStoreName(app.display, app.window, WINDOW_TITLE);

    // убираем рамки
    Atom wmHints = XInternAtom(app.display, "_MOTIF_WM_HINTS", False);
    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long inputMode;
        unsigned long status;
    } hints = {2, 0, 0, 0, 0};

    XChangeProperty(app.display, app.window, wmHints, wmHints, 32,
                    PropModeReplace, (unsigned char*)&hints, 5);

    // убираем из TASKBAR и PAGER
    Atom state = XInternAtom(app.display, "_NET_WM_STATE", False);
    //Atom skip_taskbar = XInternAtom(app.display, "_NET_WM_STATE_SKIP_TASKBAR", False);
    Atom skip_pager = XInternAtom(app.display, "_NET_WM_STATE_SKIP_PAGER", False); // Не убирает из TASKBAR, только из PAGER (из переключателя (alt+tab))

    // Atom atoms[] = { skip_taskbar/*, skip_pager*/ };
    Atom atoms[] = { /*skip_taskbar,*/ skip_pager };

    XChangeProperty(app.display, app.window, state, XA_ATOM, 32,
                PropModeReplace, (unsigned char*)atoms, 1);


    Region region = XCreateRegion();

    int width  = WINDOW_SIZE;
    int height = WINDOW_SIZE;
    int radius = width / 2;

    for (int y = 0; y < height; y++) {
        int dy = y - radius;
        int dx = (int)sqrt(radius * radius - dy * dy);

        XRectangle rect;
        rect.x = radius - dx;
        rect.y = y;
        rect.width  = dx * 2;
        rect.height = 1;

        XUnionRectWithRegion(&rect, region, region);
    }

    // применяем форму окна
    XShapeCombineRegion(app.display, app.window, ShapeBounding, 0, 0, region, ShapeSet);

    // (опционально) клики только внутри круга
    XShapeCombineRegion(app.display, app.window, ShapeInput, 0, 0, region, ShapeSet);

    // обработка событий
    XSelectInput(app.display, app.window, ExposureMask | KeyPressMask);

    XMapWindow(app.display, app.window);

    //////////////////////////////////////////////////////////////////////////
    // Инициализация Cairo с двойной буферизацией

    // 1. Xlib поверхность (прямой вывод на окно)
    app.xlib_surface = cairo_xlib_surface_create(app.display, app.window,
                                                 DefaultVisual(app.display, screen),
                                                 app.width, app.height);
    app.xlib_cr = cairo_create(app.xlib_surface);

    // 2. Off-screen изображение (буфер для отрисовки)
    app.image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, app.width, app.height);
    app.image_cr = cairo_create(app.image_surface);


    while (1) {
        while (XPending(app.display)) {
            XEvent ev;
            XNextEvent(app.display, &ev);

            if (ev.type == Expose) {
                // ПриExpose просто перерисовываем буфер и сразу выводим его.
                // XFillRectangle убран, так как он вызывал мерцание, перекрывая работу Cairo.

                // draw_clock(&app);
                // cairo_set_source_surface(app.xlib_cr, app.image_surface, 0, 0);
                // cairo_paint(app.xlib_cr);
                // cairo_surface_flush(app.xlib_surface);
            }
        }

        draw_clock(&app);

        // Копируем готовый буфер на экран одной операцией
        cairo_set_source_surface(app.xlib_cr, app.image_surface, 0, 0);
        cairo_paint(app.xlib_cr);

        cairo_surface_flush(app.xlib_surface);
        XFlush(app.display);
        usleep(1000000); // 1 second
    }

    // Cleanup (never reached)
    cairo_destroy(app.xlib_cr);
    cairo_surface_destroy(app.xlib_surface);
    cairo_destroy(app.image_cr);
    cairo_surface_destroy(app.image_surface);
    XCloseDisplay(app.display);

    return 0;
}
