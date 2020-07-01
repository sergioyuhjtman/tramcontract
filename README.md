# tramcontract
EOSIO smart contract that allows the creation of a token TRAM fully backed by RAM.

To buy TRAM, simply transfer to the contract any amount of EOS (or the corresponding blockchain CORE token). The contract will buy RAM with that amount, issue as many TRAM as bytes have been bought, and give those TRAM to the sender. The TRAM token can be freely transferred without fees. If you want to sell TRAM to the system, simply transfer those TRAM to the contract and you will receive the corresponding amount of EOS tokens. Those TRAM will be immediately burnt.
