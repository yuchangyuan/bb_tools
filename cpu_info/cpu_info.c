#include "disp.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <cairo.h>

void cpu_stat(double *user_out, double *sys_out, double *total_out)
{
    FILE *f = NULL;
    static int user, nice, sys, idle, io, irq, sirq;
    int u, n, s, i, i1, i2, i3;
    int total;

    f = fopen("/proc/stat", "r");
    fscanf(f, "cpu  %d %d %d %d %d %d %d",
           &u, &n, &s, &i, &i1, &i2, &i3);
    fclose(f);

    //printf("user, %d %d %d %d %d %d %d\n",
    //user, nice, sys, idle, io, irq, sirq);

    total = u + n + s + i + i1 + i2 + i3 -
        (user + nice + sys + idle + io + irq + sirq);
    *user_out = (double)(u - user) / total;
    *sys_out = (double)(s - sys) / total;
    *total_out = (double)(u + n + s + i1 + i2 + i3 -
                          (user + nice + sys + io + irq + sirq)) / total;

    user = u;
    nice = n;
    sys = s;
    idle = i;
    io = i1;
    irq = i2;
    sirq = i3;
}

int main()
{
    int w, h;

    uint8_t *buf;

    cairo_surface_t *surface;
    cairo_t *cr;
    float a = 0;
    uint8_t *ptr;

    buf = open_disp(&w, &h);
    // printf("buf = %p, w = %d, h = %d\n", buf, w, h);
    memset(buf, 0, w * h);

    surface = cairo_image_surface_create_for_data(buf, CAIRO_FORMAT_A8,
                                                  w, h, w);
    cr = cairo_create (surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_select_font_face(cr, "WenQuanYi Bitmap Song",
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_line_width(cr, 1);

    while (1) {
        double user, sys, total;
        char txt[256];
        int t1;
        // get
        cpu_stat(&user, &sys, &total);
        t1 = total * 1000;
        sprintf(txt, "%s%d.%d%%",
                (t1 > 999) ? "" : ((t1 > 99) ? " " : "  "),
                t1 / 10, t1 % 10);

        // clear
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_rectangle(cr, 0, 0, w, 12);
        cairo_fill(cr);

	// outline
#if 1
        cairo_set_source_rgba(cr, 1,1,1, 1);
        cairo_rectangle(cr, 0.5, 0.5, 82, 7);
        cairo_stroke(cr);
#endif

	// total
#if 1
        cairo_set_source_rgba(cr, 1,1,1, 0.25);
        cairo_rectangle(cr, 1, 1, total * 80, 6);
        cairo_fill(cr);
#endif

	// user
#if 1
        cairo_set_source_rgba(cr, 1,1,1, 1);
        cairo_rectangle(cr, 1, 1, user * 80, 6);
        cairo_fill(cr);
#endif

	// sys
#if 1
	cairo_set_source_rgba(cr, 1,1,1, 0.5);
        cairo_rectangle(cr, 1 + user * 80, 1, sys * 80, 6);
        cairo_fill(cr);
#endif
        // char
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1);
        cairo_set_font_size(cr, 12);
        cairo_move_to(cr, 84, 8);
        cairo_show_text(cr, txt);

        usleep(1000 * 1000);
    }


    close_disp();
}
