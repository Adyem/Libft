#ifndef CSV_HPP
# define CSV_HPP

#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../File/file_utils.hpp"
#include "../Parser/document_backend.hpp"
#include "../Template/vector.hpp"

class ft_csv_document
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        ft_vector<ft_string> _fields;
        ft_vector<ft_size_t> _row_offsets;
        ft_vector<ft_size_t> _row_lengths;
        char _delimiter;
        ft_bool _has_header;
        uint8_t _initialised_state;
        static thread_local int32_t _last_error;
        static int32_t set_error(int32_t error_code) noexcept;

        int32_t parse_content(const char *content, char delimiter,
            ft_bool has_header) noexcept;

    public:
        ft_csv_document() noexcept;
        ft_csv_document(const ft_csv_document &other) noexcept = delete;
        ft_csv_document(ft_csv_document &&other) noexcept = delete;
        ~ft_csv_document() noexcept;

        ft_csv_document &operator=(const ft_csv_document &other) noexcept = delete;
        ft_csv_document &operator=(ft_csv_document &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const char *content, char delimiter = ',',
            ft_bool has_header = FT_FALSE) noexcept;
        int32_t initialize(ft_document_source &source, char delimiter = ',',
            ft_bool has_header = FT_FALSE) noexcept;
        int32_t initialize_from_file(const char *file_path, char delimiter = ',',
            ft_bool has_header = FT_FALSE) noexcept;
        int32_t initialize(const ft_csv_document &other) noexcept;
        int32_t initialize(ft_csv_document &&other) noexcept;
        int32_t destroy() noexcept;
        int32_t move(ft_csv_document &other) noexcept;

        ft_size_t row_count() const noexcept;
        ft_size_t column_count(ft_size_t row_index) const noexcept;
        ft_bool has_header() const noexcept;
        char delimiter() const noexcept;
        const ft_string *get_field(ft_size_t row_index, ft_size_t column_index) const noexcept;
        const ft_string *get_header(ft_size_t column_index) const noexcept;

        int32_t write_to_string(ft_string &output) const noexcept;
        int32_t write_to_backend(ft_document_sink &sink) const noexcept;
        int32_t write_to_file(const char *file_path) const noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

int32_t csv_split_line(const char *line, ft_vector<ft_string> &fields,
    char delimiter = ',') noexcept;
char *csv_escape_field(const char *field, char delimiter = ',') noexcept;

#endif
