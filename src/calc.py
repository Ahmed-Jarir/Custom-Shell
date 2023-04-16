import string
import re
import readline

CHAR_LOOKUP = list(string.digits + string.ascii_uppercase)
current_base = "dec"
modes = {"hex": 16, "oct": 8, "bin": 2, "dec": 10}
base_cor = {"hex": "0x", "oct": "0o", "bin": "0b", "dec": ""}

def baseToDec(inp,ans):
    
    if re.search(r'\bans\b', inp):
        inp = re.sub(r'\bans\b', str(ans), inp)
    
    pattern = r"0x[\da-fA-F]+|0b[01]+|0o[0-7]+"
    matches = re.findall(pattern, inp)

    for match in matches:
        if match[:2] == '0x':
            decimal_val = str(int(match, 16))
        elif match[:2] == '0b':
            decimal_val = str(int(match, 2))
        elif match[:2] == '0o':
            decimal_val = str(int(match, 8))
        inp = inp = re.sub(r'\b'+match+r'\b', decimal_val, inp)

    return inp

def numeral(number):
    global CHAR_LOOKUP
    return CHAR_LOOKUP[number]

def convert_base(number, base):

    mods = []
    if number == 0:
        output = base_cor[current_base] + str(number)
        return output

    while number > 0:
        mods.append(numeral(number % base))
        number //= base
    mods.reverse()
    val = ''.join(mods)
    output = base_cor[current_base] + val
    return output

    

def help():
    print("Welcome to ZaCalc's Help")
    print("*******************************************************************")
    print("BASES(Modes):")
    print("There are 4 Bases(modes) which you can change to by typing it")
    print("By changing the mode the result format would change")
    print("hex  -- hexadecimal 0x.. ")
    print("oct  -- octal       0o.. ")
    print("bin  -- binary      0b.. ")
    print("dec  -- decimal       ..")
    print("Negative or float numbers are always printed in decimal by default")
    print("*******************************************************************")
    print("ans  -- latest result")
    print("exit -- program termination")
    print("base -- prints the currents base you are working on")
    

if __name__ == "__main__":

    ans = 0

    readline.parse_and_bind('"\e[A": history-search-backward')
    readline.parse_and_bind('"\e[B": history-search-forward')
    readline.parse_and_bind('"\e[D": backward-char')
    readline.parse_and_bind('"\e[C": forward-char')

    print("Welcome to ZaCalc. For instructions type help.")
    while(True):
        
        inp = input(">>> ")
        
        if inp == "exit":
            break
        
        elif inp == "help":
            help()

        elif inp == "":
            continue

        elif inp == "base":
            print(current_base) 

        elif inp in modes:
            current_base = inp
            print(f"Changed output format to {current_base}")
        
        else:
            output = baseToDec(inp,ans)
            result = eval(output)
            ans = result
            if (isinstance(result, int) or (result % 1 == 0)) and result >= 0:
                result = int(result)
                result = convert_base(result,modes[current_base])
            print(result)


            

# Referances
# https://discuss.codecademy.com/t/help-with-personal-python-project-converting-base-10-to-other-bases/39218/2