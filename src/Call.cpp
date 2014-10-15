#include <iostream>
#include <julia.h>
#include "Call.h"
#include "JuliaHandle.h"
#include "error.h"
#include "rvalue.h"
#include "lvalue.h"

using namespace std;

nj::Result nj::Call::eval(vector<shared_ptr<nj::Value>> &args)
{
   vector<shared_ptr<nj::Value>> res;
   string funcName;
   jl_module_t *mod = 0;
   int numArgs;
   int argOffset;

   if(args.size() == 0) return res;

   switch(args[0]->type()->getId())
   {
      case ascii_string_type:
      case utf8_string_type:
      {
         Primitive &funcName_r = static_cast<Primitive&>(*args[0]);
  
         funcName = funcName_r.toString();
         numArgs = args.size() - 1;
         argOffset = 1;
      }
      break;
      case julia_handle_type:
      {
         if(args.size() < 2 || !args[1]->isPrimitive()) return res;

         JuliaHandle &mod_h = static_cast<JuliaHandle&>(*args[0]);
         Primitive &funcName_r = static_cast<Primitive&>(*args[1]);

         mod = (jl_module_t*)mod_h.val();
         funcName = funcName_r.toString();
         numArgs = args.size() - 2;
         argOffset = 2;
      }
      break;
      default: return res;
   }
      
   jl_value_t *jl_res = 0;
   jl_function_t *func = 0;

   if(!mod)
   {
      func = jl_get_function(jl_core_module,funcName.c_str());
      if(!func) func = jl_get_function(jl_base_module,funcName.c_str());
      if(!func) func = jl_get_function(jl_main_module,funcName.c_str());
   }
   else func = jl_get_function(mod,funcName.c_str());

   if(!func)
   {
      shared_ptr<Exception> ex = shared_ptr<Exception>(new JuliaMethodError(string("Julia method ") + funcName + " is undefined"));
  
      return Result(ex);
   }

   if(numArgs <= 3)
   {
      switch(numArgs)
      {
         case 0: jl_res = jl_call0(func); break;
         case 1: jl_res = jl_call1(func,rvalue(args[argOffset])); break;
         case 2: jl_res = jl_call2(func,rvalue(args[argOffset]),rvalue(args[argOffset + 1])); break;
         case 3: jl_res = jl_call3(func,rvalue(args[argOffset]),rvalue(args[argOffset + 1]),rvalue(args[argOffset + 2])); break;
      }
   }
   else
   {
      jl_value_t **jl_args = new jl_value_t*[numArgs];

      for(int i = 0;i < numArgs;i++) jl_args[i] = rvalue(args[i + argOffset]);
      jl_res = jl_call(func,jl_args,numArgs);
      delete jl_args;
   }

   jl_value_t *jl_ex = jl_exception_occurred();
   
   if(jl_ex)
   {
     JL_GC_PUSH1(&jl_ex);

     shared_ptr<Exception> ex = genJuliaError(jl_ex);

     JL_GC_POP();
     return Result(ex);
   }
   else
   {
      JL_GC_PUSH1(&jl_res);
      res = lvalue(jl_res);
      JL_GC_POP();
      return Result(res);
   }
}