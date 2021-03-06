#ifndef __nj_JuAlloc
#define __nj_JuAlloc

#include "Alloc.h"
#include "Exception.h"

extern "C"
{
   typedef struct _jl_value_t jl_value_t;
};

namespace nj
{
   class JuAlloc: public Alloc
   {
      protected:

         static std::map<int64_t,JuAlloc*> pindex_index;

         int64_t _pindex;
         char *_ptr;
         size_t _len;

         JuAlloc(jl_value_t *val) throw(JuliaException);

      public:

         static std::shared_ptr<Alloc> create(jl_value_t *val) throw(JuliaException);

         virtual char *ptr() const { return _ptr; }
         virtual size_t len() const { return _len; }
         virtual int64_t pindex() const { return _pindex; }
         virtual ~JuAlloc();
   };

};

#endif
