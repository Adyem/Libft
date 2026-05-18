#!/usr/bin/env perl
use strict;
use warnings;

my $source_file = shift @ARGV;
if (!defined $source_file)
{
    $source_file = 'demo_levels.cpp';
}

open(my $source_handle, '<', $source_file)
    or die "failed to open $source_file: $!\n";
local $/ = undef;
my $source_text = <$source_handle>;
close($source_handle);

my $level_index = 0;
my $failed = 0;
while ($source_text =~ /g_demo_level_tiles_\d+\[\]\s*=\s*((?:\s*"[^"]+")+);/g)
{
    my $block = $1;
    my @rows = ($block =~ /"([^"]+)"/g);
    $level_index = $level_index + 1;
    if (validate_level($level_index, \@rows) != 0)
    {
        $failed = 1;
    }
}

if ($level_index == 0)
{
    print STDERR "no demo level tile arrays found in $source_file\n";
    exit 1;
}
if ($failed != 0)
{
    exit 1;
}
print "validated $level_index demo levels\n";
exit 0;

sub validate_level
{
    my ($level_index, $rows_ref) = @_;
    my $height = scalar(@{$rows_ref});
    my $width = 0;
    my $exit_count = 0;
    my $exit_x = -1;
    my $exit_y = -1;
    my $error_count = 0;
    my $row_index = 0;

    if ($height <= 0)
    {
        print STDERR "level $level_index has no rows\n";
        return 1;
    }
    $width = length($rows_ref->[0]);
    while ($row_index < $height)
    {
        my $row = $rows_ref->[$row_index];
        my $column_index = 0;

        if (length($row) != $width)
        {
            print STDERR "level $level_index row $row_index has width "
                . length($row) . " expected $width\n";
            $error_count = $error_count + 1;
        }
        while ($column_index < length($row))
        {
            my $tile = substr($row, $column_index, 1);
            if ($tile ne '0' && $tile ne '1' && $tile ne 'E')
            {
                print STDERR "level $level_index invalid tile '$tile' at "
                    . "$column_index,$row_index\n";
                $error_count = $error_count + 1;
            }
            if (($row_index == 0 || $row_index == $height - 1
                    || $column_index == 0 || $column_index == $width - 1)
                && $tile ne '1' && $tile ne 'E')
            {
                print STDERR "level $level_index border leak at "
                    . "$column_index,$row_index\n";
                $error_count = $error_count + 1;
            }
            if ($tile eq 'E')
            {
                $exit_count = $exit_count + 1;
                $exit_x = $column_index;
                $exit_y = $row_index;
            }
            $column_index = $column_index + 1;
        }
        $row_index = $row_index + 1;
    }
    if ($exit_count != 1)
    {
        print STDERR "level $level_index has $exit_count exits, expected 1\n";
        $error_count = $error_count + 1;
    }
    if ($error_count == 0
        && is_exit_reachable($rows_ref, $width, $height, $exit_x, $exit_y) == 0)
    {
        print STDERR "level $level_index exit is not reachable from 1,1\n";
        $error_count = $error_count + 1;
    }
    if ($error_count == 0)
    {
        print "level $level_index ok\n";
        return 0;
    }
    return 1;
}

sub is_exit_reachable
{
    my ($rows_ref, $width, $height, $exit_x, $exit_y) = @_;
    my @queue_x = (1);
    my @queue_y = (1);
    my %visited = ('1,1' => 1);
    my $head_index = 0;

    if (tile_at($rows_ref, 1, 1) eq '1')
    {
        return 0;
    }
    while ($head_index < scalar(@queue_x))
    {
        my $tile_x = $queue_x[$head_index];
        my $tile_y = $queue_y[$head_index];

        $head_index = $head_index + 1;
        if ($tile_x == $exit_x && $tile_y == $exit_y)
        {
            return 1;
        }
        enqueue_if_open(\@queue_x, \@queue_y, \%visited, $rows_ref,
            $width, $height, $tile_x + 1, $tile_y);
        enqueue_if_open(\@queue_x, \@queue_y, \%visited, $rows_ref,
            $width, $height, $tile_x - 1, $tile_y);
        enqueue_if_open(\@queue_x, \@queue_y, \%visited, $rows_ref,
            $width, $height, $tile_x, $tile_y + 1);
        enqueue_if_open(\@queue_x, \@queue_y, \%visited, $rows_ref,
            $width, $height, $tile_x, $tile_y - 1);
    }
    return 0;
}

sub enqueue_if_open
{
    my ($queue_x_ref, $queue_y_ref, $visited_ref, $rows_ref,
        $width, $height, $tile_x, $tile_y) = @_;
    my $key = "$tile_x,$tile_y";

    if ($tile_x < 0 || $tile_y < 0 || $tile_x >= $width || $tile_y >= $height)
    {
        return;
    }
    if (exists $visited_ref->{$key})
    {
        return;
    }
    if (tile_at($rows_ref, $tile_x, $tile_y) eq '1')
    {
        return;
    }
    $visited_ref->{$key} = 1;
    push(@{$queue_x_ref}, $tile_x);
    push(@{$queue_y_ref}, $tile_y);
    return;
}

sub tile_at
{
    my ($rows_ref, $tile_x, $tile_y) = @_;

    return substr($rows_ref->[$tile_y], $tile_x, 1);
}
