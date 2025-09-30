#include "lexer.h"
#include "prompt.h"
#include "io_redir.h"
#include "external_cmd.h"
#include "builtins.h"
#include "env_var_expansion.h"
#include "tilde_expansion.h"
#include "path_search.h"
#include "pipeline.h"
#include "bg_jobs.h"


int main()
{
	jobs_t *jobs = NULL;
	char* command_history[100];
	int history_count = 0;

	while (1) {
		if (jobs == NULL) jobs = jobs_init(jobs);
		jobs_check(jobs);

        print_prompt();


		/*input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();

        // Skip empty input 
        if (!input || strlen(input) == 0) {
            free(input);
            continue;
        }

        tokenlist *tokens = get_tokens(input);
        if (tokens->size == 0) {
            free(input);
            free_tokens(tokens);
            continue;
        }

		if (history_count < 100) {
			char *input_copy = malloc(strlen(input) + 1);
			if (input_copy) {
				strcpy(input_copy, input);
				command_history[history_count++] = input_copy;
			}
		}
        
        tokens = environment_variable_expansion(tokens);
        tokens = tilde_expansion(tokens);

        

        int background = 0;
        char *bg_cmdline = NULL;
        if (tokens->size > 0 && strcmp(tokens->items[tokens->size - 1], "&") == 0) {
            free(tokens->items[tokens->size - 1]);
            tokens->size -= 1;
            tokens->items[tokens->size] = NULL;
            background = 1;
            size_t n = strlen(input);
            while (n > 0 && isspace((unsigned char)input[n-1])) n--;
            if (n > 0 && input[n-1] == '&') { n--; while (n > 0 && isspace((unsigned char)input[n-1])) n--; }
            bg_cmdline = (char*)malloc(n + 1);
            if (bg_cmdline) { memcpy(bg_cmdline, input, n); bg_cmdline[n] = '\0'; }
            if (tokens->size == 0) {
                free(input);
                free_tokens(tokens);
                free(bg_cmdline);
                continue;
            }
        }

        // Check for pipes first 
        int num_commands = 0;
        tokenlist **pipe_commands = parse_pipes(tokens, &num_commands);
        
        if (num_commands > 1) {
            // Handle piped commands - find paths for all commands 
            for (int i = 0; i < num_commands; i++) {
                char *cmdpath = path_search(pipe_commands[i]->items[0]);
                if (cmdpath) {
                    free(pipe_commands[i]->items[0]);
                    pipe_commands[i]->items[0] = cmdpath;
                }
            }
            pid_t last = execute_pipeline(pipe_commands, num_commands, background);
            if (background && last > 0) {
                int jid = jobs_add(jobs, last, bg_cmdline ? bg_cmdline : input);
                if (jid > 0) printf("[%d] %d\n", jid, (int)last);
            }
            
            // Cleanup 
            for (int i = 0; i < num_commands; i++) {
                free_tokens(pipe_commands[i]);
            }
            free(pipe_commands);
        } else {
            // Handle single command with redirection 
            redir_t redir;
            if (parse_redirection(tokens, &redir) != 0) {
                // parse error: cleanup and continue 
                free(input);
                free_tokens(tokens);
                free(bg_cmdline);
                free(redir.in_file);
                free(redir.out_file);
                continue;
            }
            
            // builtin handling 
            if (strcmp(tokens->items[0], "jobs") == 0) {
                int saved_in = -1, saved_out = -1;
                if (apply_redirection(&redir, &saved_in, &saved_out) != 0) {
                    restore_stdio(saved_in, saved_out);
                    free(input);
                    free_tokens(tokens);
                    free(bg_cmdline);
                    free(redir.in_file);
                    free(redir.out_file);
                    continue;
                }
                builtin_jobs(jobs);
                restore_stdio(saved_in, saved_out);
            } else if (strcmp(tokens->items[0], "exit") == 0) {
                builtin_exit(command_history, history_count, jobs->pids, 10);
            } else if (strcmp(tokens->items[0], "echo") == 0) {
                int saved_in = -1, saved_out = -1;
                if (background) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        if (apply_redirection(&redir, NULL, NULL) != 0) _exit(1);
                        builtin_echo(tokens);
                        _exit(0);
                    } else if (pid > 0) {
                        int jid = jobs_add(jobs, pid, bg_cmdline ? bg_cmdline : input);
                        if (jid > 0) printf("[%d] %d\n", jid, (int)pid);
                    } else {
                        perror("fork failed");
                    }
                } else {
                    if (apply_redirection(&redir, &saved_in, &saved_out) != 0) {
                        // failed to apply redirection for builtin 
                        restore_stdio(saved_in, saved_out);
                        free(input);
                        free_tokens(tokens);
                        free(bg_cmdline);
                        free(redir.in_file);
                        free(redir.out_file);
                        continue;
                    }
                    builtin_echo(tokens);
                    restore_stdio(saved_in, saved_out);
                }
            } else if (strcmp(tokens->items[0], "cd") == 0) {
                    builtin_cd(tokens);
            } else {
                // external command: find path and execute; execute_external_command will apply redir in child 
                char *cmdpath = path_search(tokens->items[0]);
                if (cmdpath) {
                    free(tokens->items[0]);
                    tokens->items[0] = cmdpath;
                    pid_t pid = execute_external_command(tokens, &redir, background);
                    if (background && pid > 0) {
                        int jid = jobs_add(jobs, pid, bg_cmdline ? bg_cmdline : input);
                        if (jid > 0) printf("[%d] %d\n", jid, (int)pid);
                    }
                } else {
                    printf("Command not found\n");
                }
            }
        }
               
		free(bg_cmdline);
		free(input);
		free_tokens(tokens);
	}

	return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; // make NULL terminated 
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}


