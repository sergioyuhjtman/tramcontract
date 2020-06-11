#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

namespace ramtoken {

   class [[eosio::contract("tramcontract")]] tramcontract : public contract {
      public:
         using contract::contract;
         [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);
         [[eosio::action]] void sell(asset quantity, name account);
   
      private:
         static constexpr auto TRAM = extended_symbol(
            symbol(symbol_code("TRAM"),0), "tramcontract"_n
         );

         void buy(asset quantity, name account);

         //! account table is declared private in eosio.token.hpp
         struct account {
            asset    balance;
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };
         typedef eosio::multi_index< "accounts"_n, account > accounts;
   };
}
