#include "../../XML/xml.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>
#include <cstring>

FT_TEST(test_xml_document_thread_guard_resets_errno,
        "xml_document::thread_guard resets errno to success when locking")
{
    xml_document document;

    FT_ASSERT_EQ(true, document.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    {
        xml_document::thread_guard guard(&document);

        FT_ASSERT_EQ(0, guard.get_status());
        FT_ASSERT_EQ(true, guard.lock_acquired());
        FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_thread_guard_tolerates_null_document,
        "xml_document::thread_guard allows null documents without locking")
{
    ft_errno = FT_ERR_INTERNAL;
    xml_document::thread_guard guard(ft_nullptr);

    FT_ASSERT_EQ(0, guard.get_status());
    FT_ASSERT_EQ(false, guard.lock_acquired());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_handles_mutex_allocation_failure,
        "xml_document handles mutex allocation failures and remains usable")
{
    cma_set_alloc_limit(1);
    xml_document failure_document;
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, failure_document.get_error());
    FT_ASSERT_EQ(false, failure_document.is_thread_safe_enabled());
    ft_errno = FT_ERR_INTERNAL;
    {
        xml_document::thread_guard guard(&failure_document);

        FT_ASSERT_EQ(0, guard.get_status());
        FT_ASSERT_EQ(false, guard.lock_acquired());
        FT_ASSERT_EQ(FT_ERR_INTERNAL, ft_errno);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, failure_document.load_from_string("<root/>"));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, failure_document.get_error());
    FT_ASSERT(failure_document.get_root() != ft_nullptr);
    return (1);
}

FT_TEST(test_xml_document_write_to_string_empty_document_sets_error, "xml_document::write_to_string reports FT_ERR_INVALID_ARGUMENT when empty")
{
    xml_document document;
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = document.write_to_string();
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_empty_document_sets_error, "xml_document::write_to_file reports FT_ERR_INVALID_ARGUMENT when empty")
{
    xml_document document;
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = document.write_to_file("Test");
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_string_allocation_failure_sets_error, "xml_document::write_to_string reports FT_ERR_NO_MEMORY when allocation fails")
{
    xml_document document;
    char *result;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.load_from_string("<root/>"));
    ft_errno = FT_ERR_SUCCESSS;
    cma_set_alloc_limit(1);
    result = document.write_to_string();
    cma_set_alloc_limit(0);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, document.get_error());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_fopen_failure_sets_errno_offset, "xml_document::write_to_file reports errno on fopen failure")
{
    xml_document document;
    int result;
    int open_errno;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.load_from_string("<root/>"));
    ft_errno = FT_ERR_SUCCESSS;
    errno = 0;
    result = document.write_to_file("Test");
    open_errno = errno;
    FT_ASSERT(result != FT_ERR_SUCCESSS);
    FT_ASSERT_EQ(result, document.get_error());
    FT_ASSERT_EQ(result, ft_errno);
    if (open_errno != 0)
        FT_ASSERT_EQ(ft_map_system_error(open_errno), result);
    else
        FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_fwrite_failure_sets_errno_offset, "xml_document::write_to_file reports errno on fwrite failure")
{
    xml_document document;
    int result;
    int write_errno;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.load_from_string("<root/>"));
    ft_errno = FT_ERR_SUCCESSS;
    errno = 0;
    result = document.write_to_file("/dev/full");
    write_errno = errno;
    FT_ASSERT(result != FT_ERR_SUCCESSS);
    FT_ASSERT_EQ(result, document.get_error());
    FT_ASSERT_EQ(result, ft_errno);
    if (write_errno != 0)
        FT_ASSERT_EQ(ft_map_system_error(write_errno), result);
    else
        FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_malformed_sets_errno, "xml_document::load_from_string reports FT_ERR_INVALID_ARGUMENT on malformed input")
{
    xml_document document;
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = document.load_from_string("<root>");
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_detects_mismatched_closing_tag, "xml_document::load_from_string rejects mismatched closing tags")
{
    xml_document document;
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = document.load_from_string("<root><child></rood>");
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_rejects_trailing_characters, "xml_document::load_from_string rejects trailing content")
{
    xml_document document;
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = document.load_from_string("<root/>extra");
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, document.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT(document.get_root() == ft_nullptr);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_allocation_failure_sets_errno, "xml_document::load_from_string reports FT_ERR_NO_MEMORY on allocation failure")
{
    xml_document document;
    int result;

    cma_set_alloc_limit(1);
    ft_errno = FT_ERR_SUCCESSS;
    result = document.load_from_string("<root><child/></root>");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, document.get_error());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_success_clears_errno, "xml_document::load_from_string clears ft_errno on success")
{
    xml_document document;
    int result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = document.load_from_string("<root/>");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_handles_gt_in_attribute, "xml_document::load_from_string tolerates '>' inside attribute values")
{
    xml_document document;
    const char *xml_input;
    xml_node *root_node;
    char *written_string;

    xml_input = "<root attr=\"a>b\">payload</root>";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.load_from_string(xml_input));
    root_node = document.get_root();
    FT_ASSERT(root_node != ft_nullptr);
    FT_ASSERT(root_node->text != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(root_node->text, "payload"));
    ft_unordered_map<char*, char*>::iterator attribute_iterator = root_node->attributes.begin();
    ft_unordered_map<char*, char*>::iterator attribute_end = root_node->attributes.end();
    bool attribute_found;
    attribute_found = false;
    while (attribute_iterator != attribute_end)
    {
        if (attribute_iterator->first
            && std::strcmp(attribute_iterator->first, "attr") == 0)
        {
            attribute_found = true;
            FT_ASSERT(attribute_iterator->second != ft_nullptr);
            FT_ASSERT_EQ(0, std::strcmp(attribute_iterator->second, "a>b"));
            break ;
        }
        ++attribute_iterator;
    }
    FT_ASSERT(attribute_found);
    written_string = document.write_to_string();
    FT_ASSERT(written_string != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(written_string, "<root>payload</root>\n"));
    cma_free(written_string);
    return (1);
}

FT_TEST(test_xml_node_thread_safety_root_locking, "xml_node exposes locking helpers for root nodes")
{
    xml_document document;
    xml_node *root_node;
    bool lock_acquired;
    int lock_status;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, document.load_from_string("<root><child/></root>"));
    root_node = document.get_root();
    FT_ASSERT(root_node != ft_nullptr);
    FT_ASSERT_EQ(true, xml_node_is_thread_safe_enabled(root_node));
    ft_errno = FT_ERR_INTERNAL;
    lock_acquired = false;
    lock_status = xml_node_lock(root_node, &lock_acquired);
    FT_ASSERT_EQ(0, lock_status);
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INTERNAL, ft_errno);
    xml_node_unlock(root_node, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INTERNAL, ft_errno);
    return (1);
}

FT_TEST(test_xml_node_thread_safety_rejects_null_nodes, "xml_node locking rejects null nodes")
{
    bool lock_acquired;
    int lock_status;

    FT_ASSERT_EQ(false, xml_node_is_thread_safe_enabled(ft_nullptr));
    ft_errno = FT_ERR_SUCCESSS;
    lock_acquired = true;
    lock_status = xml_node_lock(ft_nullptr, &lock_acquired);
    FT_ASSERT_EQ(-1, lock_status);
    FT_ASSERT_EQ(false, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    xml_node_unlock(ft_nullptr, true);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

