#pragma once
#include <eosiolib/eosio.hpp>

using namespace eosio;
using std::string;

namespace ramtoken {

   class [[eosio::contract("tramcontract")]] tramcontract : public contract {
      public:
         using contract::contract;
         void on_transfer_notification(name from, name to, asset quantity, string memo);

      private:
         static constexpr auto TRAM = extended_symbol(
            symbol(symbol_code("TRAM"),0), "tramcontract"_n
         );

         void buy(asset quantity, name account);
         void settle(asset quantity, name account);

         //! account table is declared private in eosio.token.hpp
         struct account {
            asset    balance;
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };
         typedef eosio::multi_index< "accounts"_n, account > accounts;
   };
}
