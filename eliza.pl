#!/usr/bin/perl

use Chatbot::Eliza;

$bot = new Chatbot::Eliza{
    prompts_on => 0,
    memory_on => 1
};

#$bot->parse_script_data('/home/atnnn/code/eliza.txt')

$bot->command_interface();
