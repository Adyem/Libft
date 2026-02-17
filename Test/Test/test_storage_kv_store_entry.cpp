#include "../test_internal.hpp"
#include "../../Storage/kv_store.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_kv_store_entry_defaults_without_expiration,
    "kv_store_entry initializes empty payload without expiration")
{
    kv_store_entry entry;
    ft_string value_copy;
    bool has_expiration;
    long long expiration_timestamp;
    const char *value_pointer;
    bool expired;

    has_expiration = true;
    expiration_timestamp = -1;
    value_pointer = reinterpret_cast<const char *>(1);
    expired = true;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.initialize());
    FT_ASSERT_EQ(0, entry.copy_value(value_copy));
    FT_ASSERT(value_copy.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.has_expiration(has_expiration));
    FT_ASSERT_EQ(false, has_expiration);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.get_expiration(expiration_timestamp));
    FT_ASSERT_EQ(0, expiration_timestamp);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.get_value_pointer(&value_pointer));
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT(*value_pointer == '\0');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.is_expired(42, expired));
    FT_ASSERT_EQ(false, expired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);
    return (1);
}

FT_TEST(test_kv_store_entry_set_value_validates_input,
    "kv_store_entry preserves prior value when string assignment fails")
{
    kv_store_entry entry;
    ft_string copied_value;
    const char *value_pointer;

    value_pointer = ft_nullptr;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.initialize());
    FT_ASSERT_EQ(0, entry.set_value("initial"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(-1, entry.set_value(static_cast<const char *>(ft_nullptr)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.copy_value(copied_value));
    FT_ASSERT(copied_value == "initial");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.set_value(ft_string("updated")));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.get_value_pointer(&value_pointer));
    FT_ASSERT(value_pointer != ft_nullptr);
    FT_ASSERT(ft_strcmp(value_pointer, "updated") == 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);
    return (1);
}

FT_TEST(test_kv_store_entry_expiration_controls,
    "kv_store_entry manages expiration flags and validation")
{
    kv_store_entry entry;
    bool has_expiration;
    long long expiration_timestamp;
    bool expired;

    has_expiration = false;
    expiration_timestamp = 0;
    expired = false;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, entry.initialize());
    FT_ASSERT_EQ(0, entry.configure_expiration(true, 120));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.has_expiration(has_expiration));
    FT_ASSERT_EQ(true, has_expiration);
    FT_ASSERT_EQ(0, entry.get_expiration(expiration_timestamp));
    FT_ASSERT_EQ(120, expiration_timestamp);

    FT_ASSERT_EQ(0, entry.is_expired(60, expired));
    FT_ASSERT_EQ(false, expired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.is_expired(180, expired));
    FT_ASSERT_EQ(true, expired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    expired = true;
    FT_ASSERT_EQ(-1, entry.is_expired(-5, expired));
    FT_ASSERT_EQ(false, expired);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, FT_ERR_SUCCESS);

    FT_ASSERT_EQ(0, entry.configure_expiration(false, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);

    expired = true;
    FT_ASSERT_EQ(0, entry.is_expired(300, expired));
    FT_ASSERT_EQ(false, expired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, FT_ERR_SUCCESS);
    return (1);
}
