import argparse
import json
import sys
import os


class Cmd_Ut(object):

    def __init__(self):
        parser = argparse.ArgumentParser(
            description="Command Utils",
            usage="""cu <command> [<args>]
commands are:
   mostfreq     shows the most frequently used commands
   favorite     shows the your favorite commands and manipulates them
""")
        parser.add_argument("command", help="Subcommand to run")

        self.path = f"{os.path.expanduser('~')}/.config/cmd_ut/"
        if not os.path.exists(self.path):
            os.makedirs(self.path)
        args = parser.parse_args(sys.argv[1:2])
        if not hasattr(self, args.command):
            print("Unrecognized command")
            parser.print_help()
            exit(1)
        getattr(self, args.command)()

    def mostfreq(self):
        pathToJson = self.path + "mostfreq.json"
        if not os.path.exists(pathToJson):
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        def printCommands(maxCmds):
            with open(pathToJson, "r") as r:
                data = json.load(r)

            sortedData = {cmd: freq for cmd, freq in sorted(data.items(), key=lambda item: item[1], reverse=True)}
            listOfCmds = []
            index = 1
            for cmd, freq in sortedData.items() :
                print(f"{index}) {cmd} {freq}")
                listOfCmds.append(cmd)
                index += 1
                if index >= maxCmds:
                    break
            return listOfCmds

        def addCommand(command):

            with open(pathToJson, "r") as r:
                data = json.load(r)
                print(data)
                data[command] = 1 if command not in data.keys() else data[command] + 1
            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)

        def removeCommand(maxNumOfElems):

            listOfCommands = printCommands(maxNumOfElems)
            if not listOfCommands:
                print("Error: you don't have any commands in your favorites")
                return
            try:
                index = int(input("Enter Command Index:"))
            except:
                index = None

            if index is None or index > maxNumOfElems:
                print(f"Error: invalid index: {index}")
                return


            with open(pathToJson, "r") as r:
                data = json.load(r)

            del data[listOfCommands[index - 1]]

            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)




        def purge():
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        def select(maxNumberOfCommands):
            if maxNumberOfCommands == None :
                return
            listOfCommands = printCommands(maxNumberOfCommands)
            if listOfCommands == None:
                return
            index = int(input("Enter Command Index:"))
            os.system(listOfCommands[index - 1])

        parser = argparse.ArgumentParser(description="The MostFreq subcommand tracks your most used commands")
        parser.add_argument("-l", "--list-commands", dest="listCmds", help = "List a number of the most frequently used commands", nargs = '?', const = 10, type = int)
        parser.add_argument("-s", "--select", dest="select", help = "Select an index of the command that you want to run",  nargs = '?', const = 10, type = int)
        parser.add_argument("-a", "--add", dest = "command", help = "Add a commmand", default = None, type = str)
        parser.add_argument("-r", "--remove", dest = "remove", help = "Remove one of the commmands using its index",  nargs = '?', const = 10, type = int)
        parser.add_argument("-p", "--purge", dest = "purge", help = "Purge the list of the most frequently used commands", action = "store_true")

        args = parser.parse_args(sys.argv[2:])
 
        if args.listCmds:
            printCommands(args.listCmds)
        elif args.select:
            select(args.select)
        elif args.purge:
            purge()
        elif args.remove:
            removeCommand(args.remove)
        elif args.command:
            addCommand(args.command)
        else:
            print("No arguments provided. Please provide an argument.\n")
            parser.print_help()

    def favorite(self):
        pathToJson = self.path + "favorite.json"
        
        if not os.path.exists(pathToJson):
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)


        def printCommands(max_cmds):
            with open(pathToJson, "r") as r:
                data = json.load(r)

            listOfIndecies = []
            for index, cmd,  in data.items() :
                print(f"{index}) {cmd}")
                listOfIndecies.append(index)
                if int(index) >= max_cmds:
                    break
            return listOfIndecies

        def addCommand(command):

            with open(pathToJson, "r") as r:
                data = json.load(r)

            if command in data.values():
                print(f"Error: the command already exist")
                return 

            data[1 if not data else int(max(data)) + 1 ] = command

            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)

        def removeCommand(maxNumberOfElems):

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
            del data[str(index - 1)]
            with open(pathToJson, "w") as w:
                json.dump(data, w, indent = 4)


        def purge():
            with open(pathToJson, "w") as w:
                json.dump({}, w, indent = 4)

        def select(maxNumberOfCommands):
            if maxNumberOfCommands == None :
                return
            listOfCommands = printCommands(maxNumberOfCommands)
            if listOfCommands == None:
                return
            index = int(input("Enter Command Index:"))
            os.system(listOfCommands[index - 1])

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

        parser.add_argument("-l", "--list-commands", dest="listCmds", help = "List a number of the most frequently used commands", nargs = '?', const = 10, type = int)
        parser.add_argument("-s", "--select", dest="select", help = "Select an index of the command that you want to run",  nargs = '?', const = 10, type = int)
        parser.add_argument("-sp", "--swap", dest="swap", help = "Select an index of the two commands that you want to swap",  nargs = 2, const = None, type = int)
        parser.add_argument("-a", "--add", dest = "command", help = "Add a commmand", default = None, type = str)
        parser.add_argument("-r", "--remove", dest = "remove", help = "Remove one of the commmands using its index", action = "store_true")
        parser.add_argument("-p", "--purge", dest = "purge", help = "Purge the list of the most frequently used commands", action = "store_true")

        args = parser.parse_args(sys.argv[2:])
        
        if args.listCmds:
            printCommands(args.listCmds)
        elif args.select:
            select(args.select)
        elif args.swap:
            swap(args.swap)
        elif args.command:
            addCommand(args.command)
        elif args.remove:
            removeCommand(args.remove)
        elif args.purge:
            purge()
        else:
            print("No arguments provided. Please provide an argument.\n")
            parser.print_help()

if __name__ == "__main__":
    Cmd_Ut()
