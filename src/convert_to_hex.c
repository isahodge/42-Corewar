/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convert_to_hex.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sjuery <sjuery@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/02/07 19:55:29 by sjuery            #+#    #+#             */
/*   Updated: 2018/02/19 16:30:21 by ihodge           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "asm.h"

static void convert_name(t_assembler *st)
{
    int name_len;

    name_len = 1;
    while(st->line[st->i] != '"')
        st->i++;
    st->i++;
    ft_printf_fd(st->corefile, "00ea 83f3 ");
    while(st->line[st->i] != '"')
    {
        ft_printf_fd(st->corefile, "%s", ft_itoa_base(st->line[st->i], 16, 1));
        if(name_len % 12 == 0)
            ft_printf_fd(st->corefile, "\n");
        else if(name_len % 2 == 0 && st->line[st->i+2])
            ft_printf_fd(st->corefile, " ");
		st->i++;
        name_len++;
    }
}

static void convert_description(t_assembler *st)
{
    int desc_len;

    desc_len = 1;
    st->i = 0;
    while(st->line[st->i] != '"')
        st->i++;
    st->i++;
    while(st->line[st->i] != '"')
    {
        ft_printf_fd(st->corefile, "%s", ft_itoa_base(st->line[st->i], 16, 1));
        if(desc_len % 12 == 0)
            ft_printf_fd(st->corefile, "\n");
        else if(desc_len % 2 == 0 && st->line[st->i+2])
            ft_printf_fd(st->corefile, " ");
		st->i++;
        desc_len++;
    }
}

int			convert_to_hex(t_assembler *st)
{
	int		file_size;
	char	*line;

	while(get_next_line(st->sfile, &st->line))
	{
		if (ft_strstr(st->line, NAME_CMD_STRING))
			convert_name(st);
        else if (ft_strstr(st->line, COMMENT_CMD_STRING))
            convert_description(st);
		else
		{
			line = ft_strtrim(st->line);
			if (line[0] != '\0')
				parse_instructions(st);
		}
        st->i++;
	}
	while (st->label)
	{
		ft_printf("label name = %s, label offset = %i\n", st->label->name, st->label->offset);
		st->label = st->label->next;
	}
	return (1);
}
