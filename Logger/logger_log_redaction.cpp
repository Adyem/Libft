#include "logger_internal.hpp"
#include "../Basic/basic.hpp"

ft_vector<s_redaction_rule> g_redaction_rules;

static int logger_apply_redaction_rule(ft_string &text,
    const s_redaction_rule &rule)
{
    const char  *pattern;
    const char  *replacement;
    size_t      pattern_length;
    size_t      text_length;
    size_t      index;
    const char  *source;
    ft_string   result;

    if (text.last_operation_error() != FT_ERR_SUCCESS)
        return (text.last_operation_error());
    if (rule.pattern.last_operation_error() != FT_ERR_SUCCESS)
        return (rule.pattern.last_operation_error());
    if (rule.replacement.last_operation_error() != FT_ERR_SUCCESS)
        return (rule.replacement.last_operation_error());
    pattern = rule.pattern.c_str();
    if (!pattern)
        return (FT_ERR_SUCCESS);
    pattern_length = rule.pattern.size();
    if (pattern_length == 0)
        return (FT_ERR_SUCCESS);
    replacement = rule.replacement.c_str();
    text_length = text.size();
    if (text.last_operation_error() != FT_ERR_SUCCESS)
        return (text.last_operation_error());
    source = text.c_str();
    if (!source)
        source = "";
    index = 0;
    while (index < text_length)
    {
        size_t match_index;

        match_index = 0;
        while (index + match_index < text_length
            && match_index < pattern_length
            && source[index + match_index] == pattern[match_index])
            match_index++;
        if (match_index == pattern_length)
        {
            result.append(replacement);
            if (result.last_operation_error() != FT_ERR_SUCCESS)
                return (result.last_operation_error());
            index += pattern_length;
            continue ;
        }
        result.push_back(source[index]);
        if (result.last_operation_error() != FT_ERR_SUCCESS)
            return (result.last_operation_error());
        index++;
    }
    text = result;
    if (text.last_operation_error() != FT_ERR_SUCCESS)
        return (text.last_operation_error());
    return (FT_ERR_SUCCESS);
}

int ft_log_add_redaction_with_replacement(const char *pattern,
    const char *replacement)
{
    const char  *replacement_value;
    int         unlock_result;
    int         error_code;
    int         lock_error;
    s_redaction_rule rule;

    if (!pattern || !pattern[0])
    {
        return (-1);
    }
    replacement_value = replacement;
    if (!replacement_value)
        replacement_value = "[REDACTED]";
    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (-1);
    }
    rule.pattern = pattern;
    rule.replacement = replacement_value;
    if (rule.pattern.last_operation_error() != FT_ERR_SUCCESS
        || rule.replacement.last_operation_error() != FT_ERR_SUCCESS)
    {
        error_code = rule.pattern.last_operation_error();
        if (error_code == FT_ERR_SUCCESS)
            error_code = rule.replacement.last_operation_error();
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_NO_MEMORY;
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
        return (-1);
    }
    g_redaction_rules.push_back(rule);
    if (g_redaction_rules.last_operation_error() != FT_ERR_SUCCESS)
    {
        error_code = g_redaction_rules.last_operation_error();
        lock_error = logger_unlock_sinks();
        if (lock_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
        return (-1);
    }
    unlock_result = logger_unlock_sinks();
    if (unlock_result != FT_ERR_SUCCESS)
    {
        return (-1);
    }
    return (0);
}

int ft_log_add_redaction(const char *pattern)
{
    return (ft_log_add_redaction_with_replacement(pattern, "[REDACTED]"));
}

void ft_log_clear_redactions()
{
    int unlock_result;
    int clear_error;
    int lock_error;

    lock_error = logger_lock_sinks();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    g_redaction_rules.clear();
    clear_error = g_redaction_rules.last_operation_error();
    unlock_result = logger_unlock_sinks();
    if (unlock_result != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (clear_error != FT_ERR_SUCCESS)
    {
        return ;
    }
    return ;
}

int logger_copy_redaction_rules(ft_vector<s_redaction_rule> &destination)
{
    size_t rule_count;
    size_t index;
    int error_code;

    rule_count = g_redaction_rules.size();
    if (g_redaction_rules.last_operation_error() != FT_ERR_SUCCESS)
        return (g_redaction_rules.last_operation_error());
    index = 0;
    while (index < rule_count)
    {
        s_redaction_rule rule;

        rule = g_redaction_rules[index];
        if (g_redaction_rules.last_operation_error() != FT_ERR_SUCCESS)
            return (g_redaction_rules.last_operation_error());
        destination.push_back(rule);
        if (destination.last_operation_error() != FT_ERR_SUCCESS)
            return (destination.last_operation_error());
        index++;
    }
    error_code = FT_ERR_SUCCESS;
    return (error_code);
}

int logger_apply_redactions(ft_string &text,
    const ft_vector<s_redaction_rule> &rules)
{
    size_t rule_count;
    size_t index;
    int error_code;

    if (text.last_operation_error() != FT_ERR_SUCCESS)
        return (text.last_operation_error());
    rule_count = rules.size();
    if (rules.last_operation_error() != FT_ERR_SUCCESS)
        return (rules.last_operation_error());
    index = 0;
    while (index < rule_count)
    {
        s_redaction_rule rule;
        int             apply_error;

        rule = rules[index];
        if (rules.last_operation_error() != FT_ERR_SUCCESS)
            return (rules.last_operation_error());
        apply_error = logger_apply_redaction_rule(text, rule);
        if (apply_error != FT_ERR_SUCCESS)
            return (apply_error);
        index++;
    }
    error_code = FT_ERR_SUCCESS;
    return (error_code);
}
