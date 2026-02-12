#include "../test_internal.hpp"
#include "../../XML/xml.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_xml_node_copy_move_deleted,
        "xml_node copy and move operations are deleted to prevent mutex reuse")
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

FT_TEST(test_xml_document_copy_move_deleted,
        "xml_document copy and move operations are deleted to keep mutex ownership per instance")
{
    bool copy_constructible;
    bool copy_assignable;
    bool move_constructible;
    bool move_assignable;

    copy_constructible = std::is_copy_constructible<xml_document>::value;
    copy_assignable = std::is_copy_assignable<xml_document>::value;
    move_constructible = std::is_move_constructible<xml_document>::value;
    move_assignable = std::is_move_assignable<xml_document>::value;
    FT_ASSERT(!copy_constructible);
    FT_ASSERT(!copy_assignable);
    FT_ASSERT(!move_constructible);
    FT_ASSERT(!move_assignable);
    return (1);
}
