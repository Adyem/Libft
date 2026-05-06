#include "../test_internal.hpp"
#include "../../Modules/XML/xml.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <type_traits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_xml_node_copy_move_deleted)
{
    bool copy_constructible;
    bool copy_assignable;
    bool move_constructible;
    bool move_assignable;

    copy_constructible = std::is_copy_constructible<xml_node>::value;
    copy_assignable = std::is_copy_assignable<xml_node>::value;
    move_constructible = std::is_move_constructible<xml_node>::value;
    move_assignable = std::is_move_assignable<xml_node>::value;
    FT_ASSERT(!copy_constructible);
    FT_ASSERT(!copy_assignable);
    FT_ASSERT(!move_constructible);
    FT_ASSERT(!move_assignable);
    return (1);
}

FT_TEST(test_xml_document_copy_move_deleted)
{
    xml_document source_document;
    xml_document *moved_document_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        source_document.load_from_string("<root><child>value</child></root>"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.get_error());
    FT_ASSERT_EQ(FT_TRUE, source_document.is_thread_safe());

    moved_document_pointer = new xml_document(ft_move(source_document));
    FT_ASSERT(moved_document_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_TRUE, moved_document_pointer->is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_document_pointer->get_error());
    FT_ASSERT(moved_document_pointer->get_root() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_document_pointer->get_error());
    FT_ASSERT_EQ(0, ft_strcmp("root", moved_document_pointer->get_root()->name));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_document_pointer->destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_document_pointer->get_error());
    FT_ASSERT_EQ(0, ft_strcmp(moved_document_pointer->get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    delete moved_document_pointer;
    return (1);
}

FT_TEST(test_xml_document_move_into_initialized_destination_preserves_source_thread_safety)
{
    xml_document source_document;
    xml_document destination_document;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        source_document.load_from_string("<moved attr=\"x\"/>"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.get_error());
    FT_ASSERT_EQ(FT_FALSE, source_document.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination_document.load_from_string("<old/>"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.move(source_document));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.get_error());
    FT_ASSERT_EQ(FT_FALSE, destination_document.is_thread_safe());
    FT_ASSERT(destination_document.get_root() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.get_error());
    FT_ASSERT_EQ(0, ft_strcmp("moved", destination_document.get_root()->name));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_document.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_document.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(destination_document.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
