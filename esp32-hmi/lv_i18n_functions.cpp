#include "lvgl_functions.h"

#ifdef USE_I18N
#include "../ui/lv_i18n.h"

#include "../ui/vars.h"
#include "../ui/styles.h"

userLanguage language;
extern "C" userLanguage get_var_language() { return language; }
extern "C" void set_var_language(userLanguage value) { language = value; }

void translations_init()
{
    lv_i18n_init(lv_i18n_language_pack);
    translations_set_locale();
}

void translations_set_locale()
{
    // lv_style_t *style_pages = get_style_style_pages_MAIN_DEFAULT();
    // lv_style_t *style_label = get_style_style_checkbox_labels_MAIN_DEFAULT();
    // lv_style_t *style_image = get_style_style_image_bg_MAIN_DEFAULT();

    if (language == userLanguage_EN)
    {
        lv_i18n_set_locale("en");
        // if (style_pages)
        //     lv_style_set_base_dir(style_pages, LV_BASE_DIR_LTR);
        // if (style_label)
        //     lv_style_set_translate_x(style_label, 45);
        // if (style_image)
        //     lv_style_set_translate_x(style_image, 40);
    }
    else if (language == userLanguage_FA)
    {
        lv_i18n_set_locale("fa");
        // if (style_pages)
        //     lv_style_set_base_dir(style_pages, LV_BASE_DIR_RTL);
        // if (style_label)
        //     lv_style_set_translate_x(style_label, -45);
        // if (style_image)
        //     lv_style_set_translate_x(style_image, -40);
    }
    else
    {
        lv_i18n_set_locale("ar");
        // if (style_pages)
        //     lv_style_set_base_dir(style_pages, LV_BASE_DIR_RTL);
        // if (style_label)
        //     lv_style_set_translate_x(style_label, -45);
        // if (style_image)
        //     lv_style_set_translate_x(style_image, -40);
    }
}
#endif // USE_I18N
