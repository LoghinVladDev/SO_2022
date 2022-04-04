from sys import argv
from random import randint

if __name__ == '__main__':
    if len(argv) != 5:
        print('Incorrec Usage : \n'
              './generate_instructions.py <number of instruction files> <instr. count range> <max item id> <quant. range>\n'
              '     where \n'
              '         no. of instruction files    = x, where x is the number of instruction files to generate\n'
              '         instr count range           = x-y, where x is lower bound, y is upper bound : [x,y)\n'
              '         max item id                 = x, where x is the max item id\n'
              '         quant. range                = x-y, same as for instruction count\n'
              '\n'
              'Example : ./generate_instructions.py 10-100 50 50-100\n')

    file_count  = int(argv[1])
    count_range = [int(i) for i in argv[2].split('-')]
    max_item_id = int(argv[3])
    quant_range = [int(i) for i in argv[4].split('-')]
    plus_bias   = 0.6

    for instruction_file_index in range(file_count):
        with open(f'instr{instruction_file_index + 1}.txt', 'w') as instruction_file:
            for i in range(randint(count_range[0], count_range[1])):
                is_add = randint(0, 100) < (0.6 * 100)
                print(f'{randint(0, max_item_id)} '
                      f'{"+" if is_add else "-"}{randint(quant_range[0], quant_range[1])}',
                      file=instruction_file)
