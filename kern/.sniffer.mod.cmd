savedcmd_sniffer.mod := printf '%s\n'   sniffer.o | awk '!x[$$0]++ { print("./"$$0) }' > sniffer.mod
