import hashlib


def str2hash(str):
    h = hashlib.sha1(str).digest()
    out = '    {'
    out += ', '.join([ '0x{val:02x}'.format(val=c) for c in h ])
    out += '}}, //"{0}"'.format(str)
    return out



a = b"""Fear is the mind-killer.
Remember...All I'm Offering Is The Truth. Nothing More.
I fight for the Users!
Their crime is curiosity.
Roads? Where we're going, we don't need roads
Too Many Secrets
A strange game. The only winning move is not to play.
Life moves pretty fast. If you don't stop and look around once in a while, you could miss it.
The only privacy that's left is the inside of your head.
We must keep our faith in the Republic. The day we stop believing democracy can work is the day we lose it.
Well, if droids could think, there'd be none of us here, would there?
One ring to rule them all, one ring to find them, one ring the bring them all, and in the darkness bind them.
I'm sorry, Dave. I'm afraid I can't do that.
If we knew what it was we were doing, it would not be called research, would it?
What... is the air-speed velocity of an unladen swallow?
I am a leaf on the wind. Watch how I soar.
I only speak two languages: English and bad English.
We have no names, man. No names. We are nameless!
I aim to misbehave.
Curse your sudden but inevitable betrayal!"""


print ("const uint8_t nfc_hashes[][20] = {")
for l in a.split(b'\n'):
    print(str2hash(l))
print("};")
