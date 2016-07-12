#include <stdio.h>
#include <assert.h>
#include <gio/gio.h>
#include <poppler.h>

// Create HashTable from key=value pairs
GHashTable *read_args(int argc, char **argv) {
    GHashTable *table = g_hash_table_new (g_str_hash, g_str_equal);

    for (int i=0; i<argc; ++i) {
        char **strs = g_strsplit(argv[i], "=", -1);
        g_hash_table_insert(table, strs[0], strs[1]);
    }

    return table;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Usage: %s <file> field=value field2=value2 ...\n", argv[0]);
        return 1;
    }

    GError *err = NULL;
    GFile *file = g_file_new_for_path(argv[1]);
    PopplerDocument *doc = poppler_document_new_from_gfile(file, NULL, NULL, &err);
    if (err) {
        fprintf(stderr, "Error: %s", err->message);
        return 1;
    }

    GHashTable *values = read_args(argc-2, argv+2); // skip ./fill-pdf <file>

    int pages = poppler_document_get_n_pages(doc);
    for (int i=0; i<pages; ++i) {
        PopplerPage *page = poppler_document_get_page(doc, i);

        GList *forms = poppler_page_get_form_field_mapping(page);
        for (GList *l = forms; l != NULL; l = l->next) {
            PopplerFormFieldMapping *map = l->data;
            PopplerFormField *field = map->field;

            assert(poppler_form_field_get_field_type(field) == POPPLER_FORM_FIELD_TEXT);

            char *name = poppler_form_field_get_name(field);
            char *value = g_hash_table_lookup(values, name);
            printf("%s: %s\n", name, value);

            poppler_form_field_text_set_text(field, value);
        }
    }

    GFile *filled = g_file_new_for_path("./filled.pdf");
    poppler_document_save(doc, g_file_get_uri(filled), &err);
    if (err) {
        fprintf(stderr, "Error: %s", err->message);
        return 1;
    }
    
    return 0;
}
