#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <eosio/singleton.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

namespace ramtoken {

   class [[eosio::contract("tramcontract")]] tramcontract : public contract {
      public:
         using contract::contract;
         [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);
         [[eosio::action]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );
         [[eosio::action]] void open( const name& owner, const symbol& symbol, const name& ram_payer );
         [[eosio::action]] void close( const name& owner, const symbol& symbol );
         [[eosio::action]] void create( const name& issuer, const asset&  maximum_supply);
         [[eosio::action]] void issue( const name& to, const asset& quantity, const string& memo );
         [[eosio::action]] void retire( const asset& quantity, const string& memo );

      private:
         // static constexpr symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
         static constexpr auto TRAM = extended_symbol(
            symbol(symbol_code("TRAM"),0), "tramcontract"_n
         );

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         //! account table is declared private in eosio.token.hpp
         struct [[eosio::table]] account {
            asset    balance;
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::multi_index< "accounts"_n, account > accounts;
         
         void settle(asset quantity, name account);
         void buy(asset quantity, name account);
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
         void sub_balance( const name& owner, const asset& value );
   };
}
