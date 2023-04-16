import pyperclip as pc
import argparse
import json
import sys
import os


class Cmd_Ut(object):

    def __init__(self):
        # parse the subcommands
        parser = argparse.ArgumentParser(
            description="Command Utils",
            usage="""cu <command> [<args>]
commands are:
   m     shows the most frequently used commands
   f     shows the your favorite commands and manipulates them
""")
        parser.add_argument("command", help="Subcommand to run")

        # creates the path to the config file
        self.path = f"{os.path.expanduser('~')}/.config/cmd_ut/"

        # creates all the directorie in the path if any of them dont exist 
        if not os.path.exists(self.path):
            os.makedirs(self.path)
        args = parser.parse_args(sys.argv[1:2])
        # checks if the subcommand is an attribute of the class
        if not hasattr(self, args.command):
            print("Unrecognized command")
            parser.print_help()
            exit(1)
        # calls the attribute
        getattr(self, args.command)()

    def m(self):
        # sets the path to the json file
        pathToJson = self.path + "mostfreq.json"
        # creates the json file if it doesnt exist
        if not os.path.exists(pathToJson):
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        # prints the most frequently used commands sorted with respect to the usage 
        def printCommands(maxCmds):
            with open(pathToJson, "r") as r:
                data = json.load(r)

            # sorts the data with respect to the value
            sortedData = {cmd: freq for cmd, freq in sorted(data.items(), key=lambda item: item[1], reverse=True)}
            listOfCmds = []
            index = 1
            for cmd, _ in sortedData.items() :
                print(f"{index}) {cmd}")
                listOfCmds.append(cmd)
                index += 1
                if index >= maxCmds:
                    break
            # returns the list of commands sorted the same way as the printed ones
            return listOfCmds

        # adds a command to the list of command and if it exists it increases the usage
        def addCommand(command):
            print(command)
            with open(pathToJson, "r") as r:
                data = json.load(r)
            data[command] = 1 if command not in data.keys() else data[command] + 1
            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)

        def removeCommand(maxNumOfElems):

            # uses printCommands function to print the most used commands to the user and to get the sorted commands array
            listOfCommands = printCommands(maxNumOfElems)
            if not listOfCommands:
                print("Error: you don't have any commands in your favorites")
                return

            # checks if the input is valid if not the index is set to none
            try:
                index = int(input("Enter Command Index:"))
            except:
                index = None

            if index is None or index > maxNumOfElems:
                print(f"Error: invalid index: {index}")
                return


            with open(pathToJson, "r") as r:
                data = json.load(r)

            # deletes the command from data
            del data[listOfCommands[index - 1]]

            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)

        # deletes all the data in the json file
        def purge():
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        # allows the user to select a command to copy to their clipboard
        def select(maxNumberOfCommands):
            if maxNumberOfCommands == None:
                return
            listOfCommands = printCommands(maxNumberOfCommands);
            if listOfCommands == None:
                return

            # checks if the input is valid if not the index is set to none
            try:
                index = int(input("Enter Command Index:"))
            except:
                index = None

            if index is None or index > maxNumberOfCommands:
                print(f"Error: invalid index: {index}")
                return
            # copies the command selected to clipboard
            pc.copy(listOfCommands[index - 1])

        # parses flags
        parser = argparse.ArgumentParser(description="The MostFreq subcommand tracks your most used commands")
        parser.add_argument("-l", "--list-commands", dest="listCmds", help = "List a number of the most frequently used commands", nargs = '?', const = 10, type = int)
        parser.add_argument("-a", "--add", dest = "command", help = "this is only for the shell to use, it doesnt always for the custom shell user",  nargs = 1, default = None, type = str)
        parser.add_argument("-s", "--select", dest="select", help = "Select an index of the command that you want to copy", nargs = '?', const = 10, type = int)
        parser.add_argument("-r", "--remove", dest = "remove", help = "Remove one of the commmands using its index",  nargs = '?', const = 10, type = int)
        parser.add_argument("-p", "--purge", dest = "purge", help = "Purge the list of the most frequently used commands", action = "store_true")

        args = parser.parse_args(sys.argv[2:])
        # handels the parsed data 
        # only one flag can be used at a time due to the lack of time
        if args.command:
            cmd = " ".join(args.command)
            addCommand(cmd)
        elif args.listCmds:
            printCommands(args.listCmds)
        elif args.select:
            select(args.select)
        elif args.purge:
            purge()
        elif args.remove:
            removeCommand(args.remove)

        else:
            print("No arguments provided. Please provide an argument.\n")
            parser.print_help()

    def f(self):
        # sets the path to the json file

        pathToJson = self.path + "favorite.json"
        
        # creates the json file if it doesnt exist
        if not os.path.exists(pathToJson):
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        # gets the commands from the json file in a list in their respective order
        def getCmds(maxNumOfCmds):
            with open(pathToJson, "r") as r:
                data = json.load(r)
            listOfCmds = []
            for idx, cmd in data.items() :
                listOfCmds.append(cmd)
                if int(idx) >= maxNumOfCmds:
                    break
            return listOfCmds


        # prints the commands in their normal order in the file
        def printCommands(max_cmds):
            with open(pathToJson, "r") as r:
                data = json.load(r)

            listOfIndecies = []
            for index, cmd in data.items() :
                print(f"{index}) {cmd}")
                listOfIndecies.append(index)
                if int(index) >= max_cmds:
                    break
            return listOfIndecies

        def addCommand():

            with open(pathToJson, "r") as r:
                data = json.load(r)

            command = input("enter the command that you want to add: ")
            if command in data.values():
                print(f"Error: the command already exist")
                return 
            if command == "":
                return

            data[1 if not data else int(max(data)) + 1 ] = command

            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)
        def removeCommand(maxNumberOfElems):
            # resets indecies after one of the indecies was remove
            def resetIndecies(index, data):
                newData = {}
                for idx, cmd in data.items():
                    intIdx = int(idx)
                    newData[intIdx - 1 if intIdx > index else intIdx] = cmd
                return newData

            listOfCommands = printCommands(maxNumberOfElems)
            if listOfCommands == None:
                print("Error: you don't have any commands in your favorites")
                return
            try:
                index = int(input("Enter Command Index:"))
            except:
                index = None
            if index == None or index > maxNumberOfElems:
                print(f"Error: invalid index: {index}")
                return
            with open(pathToJson, "r") as r:
                data = json.load(r)
            del data[str(index)]
            data = resetIndecies(index, data)
            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)


        def purge():
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        def select(maxNumberOfCommands):
            if maxNumberOfCommands == None :
                return
            listOfCommands = getCmds(maxNumberOfCommands)
            printCommands(maxNumberOfCommands)
            if listOfCommands == None:
                return
            try:
                index = int(input("Enter Command Index:"))
            except:
                index = None

            if index == None or index > maxNumberOfCommands:
                print(f"Error: invalid index: {index}")
                return
            
            pc.copy(listOfCommands[index - 1])

        # allows the user to swap command indecies
        def swap(indeciesToSwap):
            with open(pathToJson, "r") as r:
                data = json.load(r)

            idx0 = str(indeciesToSwap[0])
            idx1 = str(indeciesToSwap[1])

            temp =  data[idx0]
            data[idx0] = data[idx1]
            data[idx1] = temp

            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)


        parser = argparse.ArgumentParser(description="The Favorite subcommand allows you to store commands that you use frequently")

        parser.add_argument("-a", "--add", dest = "command", help = "Add a commmand", action = "store_true")
        parser.add_argument("-l", "--list-commands", dest="listCmds", help = "List a number of the most frequently used commands", nargs = '?', const = 10, type = int)
        parser.add_argument("-s", "--select", dest="select", help = "Select an index of the command that you want to copy",  nargs = '?', const = 10, type = int)
        parser.add_argument("-sp", "--swap", dest="swap", help = "Select an index of the two commands that you want to swap",  nargs = 2, const = None, type = int)
        parser.add_argument("-r", "--remove", dest = "remove", help = "Remove one of the commmands using its index",  nargs = '?', const = 10, type = int)
        parser.add_argument("-p", "--purge", dest = "purge", help = "Purge the list of the most frequently used commands", action = "store_true")

        args = parser.parse_args(sys.argv[2:])

        if args.command:
            addCommand()
        elif args.listCmds:
            printCommands(args.listCmds)
        elif args.select:
            select(args.select)
        elif args.swap:
            swap(args.swap)
        elif args.remove:
            removeCommand(args.remove)
        elif args.purge:
            purge()
        else:
            print("No arguments provided. Please provide an argument.\n")
            parser.print_help()

if __name__ == "__main__":
    Cmd_Ut()
