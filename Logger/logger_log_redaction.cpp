#include "logger_internal.hpp"
#include "../Libft/libft.hpp"

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

    if (text.get_error() != FT_ERR_SUCCESSS)
        return (text.get_error());
    if (rule.pattern.get_error() != FT_ERR_SUCCESSS)
        return (rule.pattern.get_error());
    if (rule.replacement.get_error() != FT_ERR_SUCCESSS)
        return (rule.replacement.get_error());
    pattern = rule.pattern.c_str();
    if (!pattern)
        return (FT_ERR_SUCCESSS);
    pattern_length = rule.pattern.size();
    if (pattern_length == 0)
        return (FT_ERR_SUCCESSS);
    replacement = rule.replacement.c_str();
    text_length = text.size();
    if (text.get_error() != FT_ERR_SUCCESSS)
        return (text.get_error());
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
            if (result.get_error() != FT_ERR_SUCCESSS)
                return (result.get_error());
            index += pattern_length;
            continue ;
        }
        result.push_back(source[index]);
        if (result.get_error() != FT_ERR_SUCCESSS)
            return (result.get_error());
        index++;
    }
    text = result;
    if (text.get_error() != FT_ERR_SUCCESSS)
        return (text.get_error());
    return (FT_ERR_SUCCESSS);
}

int ft_log_add_redaction_with_replacement(const char *pattern,
    const char *replacement)
{
    const char  *replacement_value;
    int         unlock_result;
    int         error_code;
    s_redaction_rule rule;

    if (!pattern || !pattern[0])
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    replacement_value = replacement;
    if (!replacement_value)
        replacement_value = "[REDACTED]";
    if (logger_lock_sinks() != 0)
        return (-1);
    rule.pattern = pattern;
    rule.replacement = replacement_value;
    if (rule.pattern.get_error() != FT_ERR_SUCCESSS
        || rule.replacement.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = rule.pattern.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = rule.replacement.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_NO_MEMORY;
        if (logger_unlock_sinks() != 0)
            return (-1);
        ft_errno = error_code;
        return (-1);
    }
    g_redaction_rules.push_back(rule);
    if (g_redaction_rules.get_error() != FT_ERR_SUCCESSS)
    {
        error_code = g_redaction_rules.get_error();
        if (logger_unlock_sinks() != 0)
            return (-1);
        ft_errno = error_code;
        return (-1);
    }
    unlock_result = logger_unlock_sinks();
    if (unlock_result != 0)
        return (-1);
    ft_errno = FT_ERR_SUCCESSS;
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

    if (logger_lock_sinks() != 0)
        return ;
    g_redaction_rules.clear();
    clear_error = g_redaction_rules.get_error();
    unlock_result = logger_unlock_sinks();
    if (unlock_result != 0)
        return ;
    if (clear_error != FT_ERR_SUCCESSS)
    {
        ft_errno = clear_error;
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int logger_copy_redaction_rules(ft_vector<s_redaction_rule> &destination)
{
    size_t rule_count;
    size_t index;

    rule_count = g_redaction_rules.size();
    if (g_redaction_rules.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = g_redaction_rules.get_error();
        return (-1);
    }
    index = 0;
    while (index < rule_count)
    {
        s_redaction_rule rule;

        rule = g_redaction_rules[index];
        if (g_redaction_rules.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = g_redaction_rules.get_error();
            return (-1);
        }
        destination.push_back(rule);
        if (destination.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = destination.get_error();
            return (-1);
        }
        index++;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

int logger_apply_redactions(ft_string &text,
    const ft_vector<s_redaction_rule> &rules)
{
    size_t rule_count;
    size_t index;

    if (text.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = text.get_error();
        return (-1);
    }
    rule_count = rules.size();
    if (rules.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = rules.get_error();
        return (-1);
    }
    index = 0;
    while (index < rule_count)
    {
        s_redaction_rule rule;
        int             apply_error;

        rule = rules[index];
        if (rules.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = rules.get_error();
            return (-1);
        }
        apply_error = logger_apply_redaction_rule(text, rule);
        if (apply_error != FT_ERR_SUCCESSS)
        {
            ft_errno = apply_error;
            return (-1);
        }
        index++;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}
