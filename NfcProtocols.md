# NFC Protocols on AVR #


# LLCP #

[LLCP](http://www.nfc-forum.org/specs/spec_list/) is a connection-oriented protocol, somewhat like TCP. Our implementation is very simple, in that it supports only sending of a single data packet wrapped in an "I" Protocol Data Unit (PDU). `llcp.c` takes care of the connect  / service / lookup / disconnect sequence while the upper layer transmits the actual data packet, which can contain for example a SNEP or NPP payload.

The following diagram shows a typical LLCP conversation between an NfcSmartTag and a mobile device. Below, it depicts the simplified view from the higher level protocol, which represents a simple "Send-Ack" (for SNEP) or "Send only" (for NPP).

![http://nfc-smart-tag.googlecode.com/git/docs/LLCP.png](http://nfc-smart-tag.googlecode.com/git/docs/LLCP.png)

# SNEP #


# NPP #

# Felica Push #


# Smart Poster #