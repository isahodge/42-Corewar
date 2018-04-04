/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_bytes.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlu <mlu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/11/17 20:59:44 by mlu               #+#    #+#             */
/*   Updated: 2018/04/03 18:00:03 by ihodge           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "corewar.h"
#include "asm.h"

void	check_executing_processes(t_vm *vm, int *game_end)
{
	if (!isEmpty(vm->q))
	{
		//ft_printf("QUEUE NOT EMPTY. GAME NEVER OVER %p %p\n", vm->q->max_p, vm->q->min_p);
		*game_end = 0;
	}
	else
		ft_printf("QUEUE IS EMPTY. GAME OVER\n");
}

void	add_acb_bytes(int op, int *acb_val, int acb)
{
	if (acb == 1)
		*acb_val += REG_CODE;
	else if (acb == 2)
		*acb_val += (g_optab[op].index ? DIR_CODE : DIR_SIZE);
	else if (acb == 3)
		*acb_val += IND_SIZE;
}

void	update_pc(t_io *proc, int op, int acb)
{
	int acb_val;

	acb_val = 2;
	add_acb_bytes(op, &acb_val, ACB1(acb));
	if (g_optab[op].params >= 2)
		add_acb_bytes(op, &acb_val, ACB2(acb));
	if (g_optab[op].params == 3)
		add_acb_bytes(op, &acb_val, ACB3(acb));
	if (acb_val == 0)
		acb_val = 1;
	into_reg(VAL(PC) + acb_val, PC);
}

void	cycle_scheduler(t_vm *vm, int *counter)
{
	if (*counter == 0)
	{
		vm->checks++;
		if (vm->live >= NBR_LIVE)
			vm->cycle_to_die -= CYCLE_DELTA;
		vm->live = 0;
		reset_alive_all(vm);
	}
	*counter = (*counter + 1) % vm->cycle_to_die;
	if (vm->checks == MAX_CHECKS)
	{
		vm->cycle_to_die -= CYCLE_DELTA;
		vm->checks = 0;
	}
}

void	set_cycle_to_execute(t_vm *vm, t_io *proc)
{
	if (proc->op >= 1 && proc->op <= 16)
		proc->cycle_to_execute = g_optab[proc->op - 1].cycles + vm->cycles;
	else
		proc->cycle_to_execute = vm->cycles + 1;
}

void	process_update(t_vm *vm)
{
	int		op;
	int		previous_index;
	t_io	*proc;
	t_node	**node;

	node = &(vm->q->max_p);
	while (*node && (*node)->data->cycle_to_execute <= vm->cycles)
	{
		//ft_printf("node->priority[%i] vm->cycle[%i]\n", (*node)->priority, vm->cycles);
		proc = dequeue(vm->q);
		if (PARAM1 >= MEM_SIZE)
			into_reg(PARAM1 % MEM_SIZE, PC);
		op = proc->op;
		vis_unhighlight_process(vm, proc);
		vis_print_debug(vm);
		//ft_printf("in loop\n");
		if ((op > 0 && op < 17) && proc->cycle_to_execute == vm->cycles &&
				((g_optab[op - 1].acb &&
				  valid_acb(op - 1, vm->core[PARAM2], vm, proc)) ||
				 !g_optab[op - 1].acb) && proc->executing)
		{//execute instruction
			//ft_printf("AFTER DEQUEUE\n");
			//print_queue(vm->q);
			ft_printf("cycle[%i], op[%i] %s process[%i] pc[%i]\n", vm->cycles, op, g_optab[op - 1].opstr, proc->process + 1, PARAM1);
			previous_index = PARAM1;
			g_jt[op - 1](vm, proc);
			vm->vis[PARAM1].previous_index = previous_index;
			proc->op = vm->core[PARAM1];
			set_cycle_to_execute(vm, proc);
			enqueue(vm->q, proc, proc->executing * proc->cycle_to_execute);
			//ft_printf("AFTER ENQUE\n");
			//print_queue(vm->q);
			//ft_printf("cycle to execute [%i]\n", proc->cycle_to_execute);
			vis_highlight_process(vm, proc);
		}
		else if (op > 0 && op < 17 && proc->cycle_to_execute == vm->cycles && proc->executing)
		{//invalid instruction//update pc function(moves the pc according to acb)
			//ft_printf("INVALID; op[%i] cycle[%i]\n", op, vm->cycles);
			previous_index = PARAM1;
			update_pc(proc, op - 1, vm->core[PARAM2]);
			vm->vis[PARAM1].previous_index = previous_index;
			proc->op = vm->core[PARAM1];
			set_cycle_to_execute(vm, proc);
			enqueue(vm->q, proc, proc->executing * proc->cycle_to_execute);
			vis_highlight_process(vm, proc);
		}
		else if (proc->cycle_to_execute == vm->cycles && proc->executing)
		{//invalid opcode//move pc by 1
			//ft_printf("INVALID OP\n");
			vm->vis[PARAM2].previous_index = PARAM1;
			into_reg(VAL(PC) + 1, PC);
			proc->op = vm->core[PARAM1];
			set_cycle_to_execute(vm, proc);
			enqueue(vm->q, proc, proc->executing * proc->cycle_to_execute);
			vis_highlight_process(vm, proc);
		}
		else if (!proc->executing && proc->cycle_to_execute == 0)
		{//dead process//decrease process count
			ft_printf("DEAD PROCESS\n");
		}
		//ft_printf("node->priority[%i] vm->cycles[%i]\n", (*node)->priority, vm->cycles);
	}
}

void	read_bytes(t_vm *vm, int game_end, int counter)
{
	while (1)
	{
		vm->cycles++;
		process_update(vm);
		if (vm->f.g)
		{
			refresh();
			usleep(50000);
		}
		//ft_printf("cycle[%i]\n", vm->cycles);
		check_executing_processes(vm, &game_end);
		cycle_scheduler(vm, &counter);
		if (game_end || vm->cycles == 6000)
			break ;
		game_end = 1;
	}
	endwin();
	ft_printf("\nContestant %i, \"%s\", has won ! CTD[%i] cycle[%i] process_count[%i]\n", vm->win_player,
		vm->head[vm->win_player - 1].prog_name, vm->cycle_to_die, vm->cycles, vm->process_count);
}
