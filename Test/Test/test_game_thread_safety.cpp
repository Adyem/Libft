#include "../../Game/game_achievement.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_equipment.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_skill.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../PThread/thread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"

static void run_buff_increment_task(ft_buff *buff, int iterations)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        buff->add_modifier1(1);
        index++;
    }
    return ;
}

static void run_debuff_increment_task(ft_debuff *debuff, int iterations)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        debuff->add_modifier1(1);
        index++;
    }
    return ;
}

static void run_equipment_equip_task(ft_equipment *equipment, int slot,
        ft_sharedptr<ft_item> item, int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        if (equipment->equip(slot, item) != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_skill_add_cooldown_task(ft_skill *skill, int iterations)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        skill->add_cooldown(1);
        index++;
    }
    return ;
}

static void run_buff_assignment_task(ft_buff *destination, ft_buff *source,
        int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_debuff_assignment_task(ft_debuff *destination,
        ft_debuff *source, int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_skill_assignment_task(ft_skill *destination, ft_skill *source,
        int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_equipment_assignment_task(ft_equipment *destination,
        ft_equipment *source, int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_upgrade_increment_task(ft_upgrade *upgrade, int iterations)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        upgrade->add_modifier1(1);
        index++;
    }
    return ;
}

static void run_upgrade_assignment_task(ft_upgrade *destination,
        ft_upgrade *source, int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_achievement_progress_task(ft_achievement *achievement, int goal_id,
        int iterations, int increment, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        achievement->add_progress(goal_id, increment);
        if (achievement->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

static void run_achievement_assignment_task(ft_achievement *destination,
        ft_achievement *source, int iterations, int *failure_flag)
{
    int index;

    index = 0;
    while (index < iterations)
    {
        *destination = *source;
        if (destination->get_error() != ER_SUCCESS)
        {
            *failure_flag = 1;
            return ;
        }
        index++;
    }
    *failure_flag = 0;
    return ;
}

FT_TEST(test_game_buff_concurrent_modifier_updates,
        "ft_buff modifier updates remain consistent under concurrency")
{
    ft_buff buff;
    ft_thread thread_one;
    ft_thread thread_two;
    int iterations;

    iterations = 2048;
    ft_errno = ER_SUCCESS;
    buff.set_modifier1(0);
    thread_one = ft_thread(run_buff_increment_task, &buff, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two = ft_thread(run_buff_increment_task, &buff, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    thread_one.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    FT_ASSERT_EQ(iterations * 2, buff.get_modifier1());
    FT_ASSERT_EQ(ER_SUCCESS, buff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_debuff_concurrent_modifier_updates,
        "ft_debuff modifier updates remain consistent under concurrency")
{
    ft_debuff debuff;
    ft_thread thread_one;
    ft_thread thread_two;
    int iterations;

    iterations = 1024;
    ft_errno = ER_SUCCESS;
    debuff.set_modifier1(0);
    thread_one = ft_thread(run_debuff_increment_task, &debuff, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two = ft_thread(run_debuff_increment_task, &debuff, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    thread_one.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    FT_ASSERT_EQ(iterations * 2, debuff.get_modifier1());
    FT_ASSERT_EQ(ER_SUCCESS, debuff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_equipment_concurrent_equips,
        "ft_equipment equip operations remain mutex protected")
{
    ft_equipment equipment;
    ft_sharedptr<ft_item> head_item;
    ft_sharedptr<ft_item> chest_item;
    ft_thread head_thread;
    ft_thread chest_thread;
    int head_failure_flag;
    int chest_failure_flag;
    int iterations;

    iterations = 256;
    head_failure_flag = 0;
    chest_failure_flag = 0;
    ft_errno = ER_SUCCESS;
    head_item = ft_sharedptr<ft_item>(new ft_item());
    chest_item = ft_sharedptr<ft_item>(new ft_item());
    FT_ASSERT(head_item.get() != ft_nullptr);
    FT_ASSERT(chest_item.get() != ft_nullptr);
    head_item->set_item_id(1);
    chest_item->set_item_id(2);
    head_thread = ft_thread(run_equipment_equip_task, &equipment, EQUIP_HEAD,
            head_item, iterations, &head_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, head_thread.get_error());
    chest_thread = ft_thread(run_equipment_equip_task, &equipment, EQUIP_CHEST,
            chest_item, iterations, &chest_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, chest_thread.get_error());
    head_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, head_thread.get_error());
    chest_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, chest_thread.get_error());
    FT_ASSERT_EQ(0, head_failure_flag);
    FT_ASSERT_EQ(0, chest_failure_flag);
    FT_ASSERT(equipment.get_item(EQUIP_HEAD).get() == head_item.get());
    FT_ASSERT(equipment.get_item(EQUIP_CHEST).get() == chest_item.get());
    FT_ASSERT_EQ(ER_SUCCESS, equipment.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_skill_concurrent_cooldown_updates,
        "ft_skill cooldown updates remain consistent under concurrency")
{
    ft_skill skill;
    ft_thread add_thread_one;
    ft_thread add_thread_two;
    int iterations;

    iterations = 256;
    ft_errno = ER_SUCCESS;
    skill.set_cooldown(0);
    add_thread_one = ft_thread(run_skill_add_cooldown_task, &skill, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, add_thread_one.get_error());
    add_thread_two = ft_thread(run_skill_add_cooldown_task, &skill, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, add_thread_two.get_error());
    add_thread_one.join();
    FT_ASSERT_EQ(ER_SUCCESS, add_thread_one.get_error());
    add_thread_two.join();
    FT_ASSERT_EQ(ER_SUCCESS, add_thread_two.get_error());
    FT_ASSERT_EQ(iterations * 2, skill.get_cooldown());
    FT_ASSERT_EQ(ER_SUCCESS, skill.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    skill.sub_cooldown(skill.get_cooldown());
    FT_ASSERT_EQ(0, skill.get_cooldown());
    FT_ASSERT_EQ(ER_SUCCESS, skill.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_upgrade_concurrent_modifier_updates,
        "ft_upgrade modifier updates remain consistent under concurrency")
{
    ft_upgrade upgrade;
    ft_thread thread_one;
    ft_thread thread_two;
    int iterations;

    iterations = 1024;
    ft_errno = ER_SUCCESS;
    upgrade.set_modifier1(0);
    thread_one = ft_thread(run_upgrade_increment_task, &upgrade, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two = ft_thread(run_upgrade_increment_task, &upgrade, iterations);
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    thread_one.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    FT_ASSERT_EQ(iterations * 2, upgrade.get_modifier1());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_achievement_concurrent_progress_updates,
        "ft_achievement progress updates remain consistent under concurrency")
{
    ft_achievement achievement;
    ft_thread thread_one;
    ft_thread thread_two;
    int failure_one;
    int failure_two;
    int iterations;
    int goal_id;

    iterations = 256;
    goal_id = 42;
    failure_one = 0;
    failure_two = 0;
    ft_errno = ER_SUCCESS;
    achievement.set_goal(goal_id, iterations * 2);
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    thread_one = ft_thread(run_achievement_progress_task, &achievement, goal_id,
            iterations, 1, &failure_one);
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two = ft_thread(run_achievement_progress_task, &achievement, goal_id,
            iterations, 1, &failure_two);
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    thread_one.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_one.get_error());
    thread_two.join();
    FT_ASSERT_EQ(ER_SUCCESS, thread_two.get_error());
    FT_ASSERT_EQ(0, failure_one);
    FT_ASSERT_EQ(0, failure_two);
    FT_ASSERT_EQ(iterations * 2, achievement.get_progress(goal_id));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_buff_cross_assignment_deadlock_free,
        "ft_buff assignment avoids deadlock and preserves mutex ordering")
{
    ft_buff left_buff;
    ft_buff right_buff;
    ft_thread left_thread;
    ft_thread right_thread;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    ft_errno = ER_SUCCESS;
    iterations = 256;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_buff.set_id(11);
    left_buff.set_duration(64);
    left_buff.set_modifier1(7);
    left_buff.set_modifier2(3);
    left_buff.set_modifier3(9);
    left_buff.set_modifier4(5);
    right_buff.set_id(22);
    right_buff.set_duration(128);
    right_buff.set_modifier1(2);
    right_buff.set_modifier2(4);
    right_buff.set_modifier3(6);
    right_buff.set_modifier4(8);
    left_thread = ft_thread(run_buff_assignment_task, &left_buff, &right_buff,
            iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_buff_assignment_task, &right_buff, &left_buff,
            iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT_EQ(left_buff.get_id(), right_buff.get_id());
    FT_ASSERT_EQ(left_buff.get_duration(), right_buff.get_duration());
    FT_ASSERT_EQ(left_buff.get_modifier1(), right_buff.get_modifier1());
    FT_ASSERT_EQ(left_buff.get_modifier2(), right_buff.get_modifier2());
    FT_ASSERT_EQ(left_buff.get_modifier3(), right_buff.get_modifier3());
    FT_ASSERT_EQ(left_buff.get_modifier4(), right_buff.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, left_buff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_buff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_debuff_cross_assignment_deadlock_free,
        "ft_debuff assignment avoids deadlock and preserves mutex ordering")
{
    ft_debuff left_debuff;
    ft_debuff right_debuff;
    ft_thread left_thread;
    ft_thread right_thread;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    ft_errno = ER_SUCCESS;
    iterations = 256;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_debuff.set_id(13);
    left_debuff.set_duration(42);
    left_debuff.set_modifier1(5);
    left_debuff.set_modifier2(7);
    left_debuff.set_modifier3(11);
    left_debuff.set_modifier4(13);
    right_debuff.set_id(26);
    right_debuff.set_duration(84);
    right_debuff.set_modifier1(17);
    right_debuff.set_modifier2(19);
    right_debuff.set_modifier3(23);
    right_debuff.set_modifier4(29);
    left_thread = ft_thread(run_debuff_assignment_task, &left_debuff,
            &right_debuff, iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_debuff_assignment_task, &right_debuff,
            &left_debuff, iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT_EQ(left_debuff.get_id(), right_debuff.get_id());
    FT_ASSERT_EQ(left_debuff.get_duration(), right_debuff.get_duration());
    FT_ASSERT_EQ(left_debuff.get_modifier1(), right_debuff.get_modifier1());
    FT_ASSERT_EQ(left_debuff.get_modifier2(), right_debuff.get_modifier2());
    FT_ASSERT_EQ(left_debuff.get_modifier3(), right_debuff.get_modifier3());
    FT_ASSERT_EQ(left_debuff.get_modifier4(), right_debuff.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, left_debuff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_debuff.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_skill_cross_assignment_deadlock_free,
        "ft_skill assignment avoids deadlock and preserves mutex ordering")
{
    ft_skill left_skill;
    ft_skill right_skill;
    ft_thread left_thread;
    ft_thread right_thread;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    ft_errno = ER_SUCCESS;
    iterations = 256;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_skill.set_id(31);
    left_skill.set_level(9);
    left_skill.set_cooldown(17);
    left_skill.set_modifier1(4);
    left_skill.set_modifier2(8);
    left_skill.set_modifier3(12);
    left_skill.set_modifier4(16);
    right_skill.set_id(62);
    right_skill.set_level(27);
    right_skill.set_cooldown(33);
    right_skill.set_modifier1(20);
    right_skill.set_modifier2(24);
    right_skill.set_modifier3(28);
    right_skill.set_modifier4(32);
    left_thread = ft_thread(run_skill_assignment_task, &left_skill,
            &right_skill, iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_skill_assignment_task, &right_skill,
            &left_skill, iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT_EQ(left_skill.get_id(), right_skill.get_id());
    FT_ASSERT_EQ(left_skill.get_level(), right_skill.get_level());
    FT_ASSERT_EQ(left_skill.get_cooldown(), right_skill.get_cooldown());
    FT_ASSERT_EQ(left_skill.get_modifier1(), right_skill.get_modifier1());
    FT_ASSERT_EQ(left_skill.get_modifier2(), right_skill.get_modifier2());
    FT_ASSERT_EQ(left_skill.get_modifier3(), right_skill.get_modifier3());
    FT_ASSERT_EQ(left_skill.get_modifier4(), right_skill.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, left_skill.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_skill.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_equipment_cross_assignment_deadlock_free,
        "ft_equipment assignment avoids deadlock and preserves mutex ordering")
{
    ft_equipment left_equipment;
    ft_equipment right_equipment;
    ft_thread left_thread;
    ft_thread right_thread;
    ft_sharedptr<ft_item> left_head_item;
    ft_sharedptr<ft_item> left_chest_item;
    ft_sharedptr<ft_item> left_weapon_item;
    ft_sharedptr<ft_item> right_head_item;
    ft_sharedptr<ft_item> right_chest_item;
    ft_sharedptr<ft_item> right_weapon_item;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    iterations = 128;
    ft_errno = ER_SUCCESS;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_head_item = ft_sharedptr<ft_item>(new ft_item());
    left_chest_item = ft_sharedptr<ft_item>(new ft_item());
    left_weapon_item = ft_sharedptr<ft_item>(new ft_item());
    right_head_item = ft_sharedptr<ft_item>(new ft_item());
    right_chest_item = ft_sharedptr<ft_item>(new ft_item());
    right_weapon_item = ft_sharedptr<ft_item>(new ft_item());
    FT_ASSERT(left_head_item.get() != ft_nullptr);
    FT_ASSERT(left_chest_item.get() != ft_nullptr);
    FT_ASSERT(left_weapon_item.get() != ft_nullptr);
    FT_ASSERT(right_head_item.get() != ft_nullptr);
    FT_ASSERT(right_chest_item.get() != ft_nullptr);
    FT_ASSERT(right_weapon_item.get() != ft_nullptr);
    left_head_item->set_item_id(101);
    left_chest_item->set_item_id(102);
    left_weapon_item->set_item_id(103);
    right_head_item->set_item_id(201);
    right_chest_item->set_item_id(202);
    right_weapon_item->set_item_id(203);
    FT_ASSERT_EQ(ER_SUCCESS, left_equipment.equip(EQUIP_HEAD, left_head_item));
    FT_ASSERT_EQ(ER_SUCCESS, left_equipment.equip(EQUIP_CHEST,
            left_chest_item));
    FT_ASSERT_EQ(ER_SUCCESS, left_equipment.equip(EQUIP_WEAPON,
            left_weapon_item));
    FT_ASSERT_EQ(ER_SUCCESS, right_equipment.equip(EQUIP_HEAD,
            right_head_item));
    FT_ASSERT_EQ(ER_SUCCESS, right_equipment.equip(EQUIP_CHEST,
            right_chest_item));
    FT_ASSERT_EQ(ER_SUCCESS, right_equipment.equip(EQUIP_WEAPON,
            right_weapon_item));
    left_thread = ft_thread(run_equipment_assignment_task, &left_equipment,
            &right_equipment, iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_equipment_assignment_task, &right_equipment,
            &left_equipment, iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT(left_equipment.get_item(EQUIP_HEAD).get() ==
            right_equipment.get_item(EQUIP_HEAD).get());
    FT_ASSERT(left_equipment.get_item(EQUIP_CHEST).get() ==
            right_equipment.get_item(EQUIP_CHEST).get());
    FT_ASSERT(left_equipment.get_item(EQUIP_WEAPON).get() ==
            right_equipment.get_item(EQUIP_WEAPON).get());
    FT_ASSERT_EQ(ER_SUCCESS, left_equipment.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_equipment.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_upgrade_cross_assignment_deadlock_free,
        "ft_upgrade assignment avoids deadlock and preserves mutex ordering")
{
    ft_upgrade left_upgrade;
    ft_upgrade right_upgrade;
    ft_thread left_thread;
    ft_thread right_thread;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    ft_errno = ER_SUCCESS;
    iterations = 256;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_upgrade.set_id(5);
    left_upgrade.set_current_level(3);
    left_upgrade.set_max_level(10);
    left_upgrade.set_modifier1(11);
    left_upgrade.set_modifier2(13);
    left_upgrade.set_modifier3(17);
    left_upgrade.set_modifier4(19);
    right_upgrade.set_id(7);
    right_upgrade.set_current_level(9);
    right_upgrade.set_max_level(20);
    right_upgrade.set_modifier1(23);
    right_upgrade.set_modifier2(29);
    right_upgrade.set_modifier3(31);
    right_upgrade.set_modifier4(37);
    left_thread = ft_thread(run_upgrade_assignment_task, &left_upgrade,
            &right_upgrade, iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_upgrade_assignment_task, &right_upgrade,
            &left_upgrade, iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT_EQ(left_upgrade.get_id(), right_upgrade.get_id());
    FT_ASSERT_EQ(ER_SUCCESS, left_upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_upgrade.get_error());
    FT_ASSERT_EQ(left_upgrade.get_current_level(), right_upgrade.get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, left_upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_upgrade.get_error());
    FT_ASSERT_EQ(left_upgrade.get_max_level(), right_upgrade.get_max_level());
    FT_ASSERT_EQ(ER_SUCCESS, left_upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_upgrade.get_error());
    FT_ASSERT_EQ(left_upgrade.get_modifier1(), right_upgrade.get_modifier1());
    FT_ASSERT_EQ(left_upgrade.get_modifier2(), right_upgrade.get_modifier2());
    FT_ASSERT_EQ(left_upgrade.get_modifier3(), right_upgrade.get_modifier3());
    FT_ASSERT_EQ(left_upgrade.get_modifier4(), right_upgrade.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, left_upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_upgrade.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_game_achievement_cross_assignment_deadlock_free,
        "ft_achievement assignment avoids deadlock and preserves mutex ordering")
{
    ft_achievement left_achievement;
    ft_achievement right_achievement;
    ft_thread left_thread;
    ft_thread right_thread;
    int iterations;
    int left_failure_flag;
    int right_failure_flag;

    ft_errno = ER_SUCCESS;
    iterations = 256;
    left_failure_flag = -1;
    right_failure_flag = -1;
    left_achievement.set_id(90);
    left_achievement.set_goal(1, 10);
    left_achievement.set_progress(1, 4);
    left_achievement.set_goal(2, 20);
    left_achievement.set_progress(2, 8);
    left_achievement.set_goal(3, 12);
    left_achievement.set_progress(3, 6);
    right_achievement.set_id(180);
    right_achievement.set_goal(1, 30);
    right_achievement.set_progress(1, 12);
    right_achievement.set_goal(2, 14);
    right_achievement.set_progress(2, 7);
    right_achievement.set_goal(3, 15);
    right_achievement.set_progress(3, 5);
    left_thread = ft_thread(run_achievement_assignment_task, &left_achievement,
            &right_achievement, iterations, &left_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread = ft_thread(run_achievement_assignment_task, &right_achievement,
            &left_achievement, iterations, &right_failure_flag);
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    left_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, left_thread.get_error());
    right_thread.join();
    FT_ASSERT_EQ(ER_SUCCESS, right_thread.get_error());
    FT_ASSERT_EQ(0, left_failure_flag);
    FT_ASSERT_EQ(0, right_failure_flag);
    FT_ASSERT_EQ(left_achievement.get_id(), right_achievement.get_id());
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_goal(1), right_achievement.get_goal(1));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_progress(1), right_achievement.get_progress(1));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_goal(2), right_achievement.get_goal(2));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_progress(2), right_achievement.get_progress(2));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_goal(3), right_achievement.get_goal(3));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.get_progress(3), right_achievement.get_progress(3));
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(left_achievement.is_complete(), right_achievement.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, left_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, right_achievement.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
