#include "readline_internal.hpp"

s_readline_state::s_readline_state(void)
{
    int32_t index;

    this->buffer = ft_nullptr;
    this->buffer_size = 0;
    this->position = 0;
    this->prev_buffer_length = 0;
    this->display_pos = 0;
    this->prev_display_columns = 0;
    this->history_index = 0;
    this->in_completion_mode = 0;
    this->current_match_count = 0;
    this->current_match_index = 0;
    this->word_start = 0;
    index = 0;
    while (index < MAX_SUGGESTIONS)
    {
        this->current_matches[index] = ft_nullptr;
        this->current_match_scores[index] = 0;
        index++;
    }
    this->mutex = ft_nullptr;
    return ;
}

s_readline_state::~s_readline_state(void)
{
    int32_t index;

    this->buffer = ft_nullptr;
    this->buffer_size = 0;
    this->position = 0;
    this->prev_buffer_length = 0;
    this->display_pos = 0;
    this->prev_display_columns = 0;
    this->history_index = 0;
    this->in_completion_mode = 0;
    this->current_match_count = 0;
    this->current_match_index = 0;
    this->word_start = 0;
    index = 0;
    while (index < MAX_SUGGESTIONS)
    {
        this->current_matches[index] = ft_nullptr;
        this->current_match_scores[index] = 0;
        index++;
    }
    this->mutex = ft_nullptr;
    return ;
}
