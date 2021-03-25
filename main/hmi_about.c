// The author disclaims copyright to this source code.

#include "hmi_about.h"

/** about page content */
static lv_obj_t *hmi_label_about;

lv_obj_t* hmi_about_create_tab(lv_obj_t *parent)
{

    lv_obj_t *tab = lv_tabview_add_tab(parent, "About");

    hmi_label_about = lv_label_create(tab, NULL);
    lv_label_set_long_mode(hmi_label_about, LV_LABEL_LONG_BREAK);
    lv_label_set_text(hmi_label_about,
            "LICENSE\n"
                    "\n"
                    "KWEKER - As useful as windshield wipers on a goat's ass.\n"
                    "The Unlicense\n"
                    "This is free and unencumbered software released into the public domain.\n"
                    "\n"
                    "\n"
                    "3RD PARTY LICENSES\n"
                    "\n"
                    "LVGL - Light and Versatile Graphics Library\n"
                    "MIT licence\n"
                    "Copyright (c) 2020 LVGL LLC\n"
                    "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n"
                    "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n"
                    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n");
    lv_obj_set_size(hmi_label_about, lv_page_get_scrl_width(tab) - 20,
            lv_obj_get_height(tab));
    lv_obj_set_pos(hmi_label_about, 5, 5);
    return tab;
}

