#include "../../XML/xml.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>

FT_TEST(test_xml_document_write_to_string_empty_document_sets_error, "xml_document::write_to_string reports FT_EINVAL when empty")
{
    xml_document document;
    char *result;

    ft_errno = ER_SUCCESS;
    result = document.write_to_string();
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, document.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_empty_document_sets_error, "xml_document::write_to_file reports FT_EINVAL when empty")
{
    xml_document document;
    int result;

    ft_errno = ER_SUCCESS;
    result = document.write_to_file("Test");
    FT_ASSERT_EQ(FT_EINVAL, result);
    FT_ASSERT_EQ(FT_EINVAL, document.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_string_allocation_failure_sets_error, "xml_document::write_to_string reports FT_EALLOC when allocation fails")
{
    xml_document document;
    char *result;

    FT_ASSERT_EQ(ER_SUCCESS, document.load_from_string("<root/>"));
    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    result = document.write_to_string();
    cma_set_alloc_limit(0);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(FT_EALLOC, document.get_error());
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_fopen_failure_sets_errno_offset, "xml_document::write_to_file reports errno on fopen failure")
{
    xml_document document;
    int result;
    int open_errno;

    FT_ASSERT_EQ(ER_SUCCESS, document.load_from_string("<root/>"));
    ft_errno = ER_SUCCESS;
    errno = 0;
    result = document.write_to_file("Test");
    open_errno = errno;
    FT_ASSERT(result != ER_SUCCESS);
    FT_ASSERT_EQ(result, document.get_error());
    FT_ASSERT_EQ(result, ft_errno);
    if (open_errno != 0)
        FT_ASSERT_EQ(open_errno + ERRNO_OFFSET, result);
    else
        FT_ASSERT_EQ(FT_EINVAL, result);
    return (1);
}

FT_TEST(test_xml_document_write_to_file_fwrite_failure_sets_errno_offset, "xml_document::write_to_file reports errno on fwrite failure")
{
    xml_document document;
    int result;
    int write_errno;

    FT_ASSERT_EQ(ER_SUCCESS, document.load_from_string("<root/>"));
    ft_errno = ER_SUCCESS;
    errno = 0;
    result = document.write_to_file("/dev/full");
    write_errno = errno;
    FT_ASSERT(result != ER_SUCCESS);
    FT_ASSERT_EQ(result, document.get_error());
    FT_ASSERT_EQ(result, ft_errno);
    if (write_errno != 0)
        FT_ASSERT_EQ(write_errno + ERRNO_OFFSET, result);
    else
        FT_ASSERT_EQ(FT_EINVAL, result);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_malformed_sets_errno, "xml_document::load_from_string reports FT_EINVAL on malformed input")
{
    xml_document document;
    int result;

    ft_errno = ER_SUCCESS;
    result = document.load_from_string("<root>");
    FT_ASSERT_EQ(FT_EINVAL, result);
    FT_ASSERT_EQ(FT_EINVAL, document.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_detects_mismatched_closing_tag, "xml_document::load_from_string rejects mismatched closing tags")
{
    xml_document document;
    int result;

    ft_errno = ER_SUCCESS;
    result = document.load_from_string("<root><child></rood>");
    FT_ASSERT_EQ(FT_EINVAL, result);
    FT_ASSERT_EQ(FT_EINVAL, document.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_rejects_trailing_characters, "xml_document::load_from_string rejects trailing content")
{
    xml_document document;
    int result;

    ft_errno = ER_SUCCESS;
    result = document.load_from_string("<root/>extra");
    FT_ASSERT_EQ(FT_EINVAL, result);
    FT_ASSERT_EQ(FT_EINVAL, document.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT(document.get_root() == ft_nullptr);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_allocation_failure_sets_errno, "xml_document::load_from_string reports FT_EALLOC on allocation failure")
{
    xml_document document;
    int result;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    result = document.load_from_string("<root><child/></root>");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_EALLOC, result);
    FT_ASSERT_EQ(FT_EALLOC, document.get_error());
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_xml_document_load_from_string_success_clears_errno, "xml_document::load_from_string clears ft_errno on success")
{
    xml_document document;
    int result;

    ft_errno = FT_EINVAL;
    result = document.load_from_string("<root/>");
    FT_ASSERT_EQ(ER_SUCCESS, result);
    FT_ASSERT_EQ(ER_SUCCESS, document.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

