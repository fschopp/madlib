#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>
#include <miscadmin.h>
#include <access/htup.h>
#include <catalog/pg_proc.h>
#include <utils/acl.h>
#include <utils/builtins.h>
#include <utils/hsearch.h>
#include <utils/syscache.h>

typedef struct {
    Oid         oid; /* hash key must be first*/
    FmgrInfo    info;
} CallOidHashTabEntry;

PG_FUNCTION_INFO_V1(call_oid);
Datum
call_oid(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0))
        PG_RETURN_NULL();
    
    Oid funcOid = PG_GETARG_OID(0);
    FunctionCallInfoData funcPtrCallInfo;
    HTAB **cache = (HTAB**) (fcinfo->flinfo->fn_retset
        ? &((FuncCallContext*) fcinfo->flinfo->fn_extra)->user_fctx
        : &fcinfo->flinfo->fn_extra);
    MemoryContext hashContext = fcinfo->flinfo->fn_retset
        ? ((FuncCallContext*) fcinfo->flinfo->fn_extra)->multi_call_memory_ctx
        : fcinfo->flinfo->fn_mcxt;
    
    /*
	 * We arrange to look up info about the function only once per series of
	 * calls, assuming the function info doesn't change underneath us.
	 */
    if (*cache == NULL) {
        HASHCTL ctl;
        ctl.keysize = sizeof(Oid);
        ctl.entrysize = sizeof(CallOidHashTabEntry);
        ctl.hash = oid_hash;
        ctl.hcxt = hashContext;
        *cache = hash_create(
            "call_oid hash table", /* tabname -- a name for the table (for debugging purposes) */
            10, /* nelem -- maximum number of elements expected */
            &ctl, /* info: additional table parameters, as indicated by flags */
            HASH_ELEM | HASH_FUNCTION | HASH_CONTEXT /* flags -- bitmask indicating which parameters to take from *info */
        );
    }
    
    CallOidHashTabEntry *cachedFn;
    bool found;
    cachedFn = (CallOidHashTabEntry*) hash_search(*cache, &funcOid, HASH_ENTER, &found);
    /* cachedFn.oid is already set */
    if (!found) {
        fmgr_info_cxt(funcOid, &cachedFn->info, hashContext);
        
        /* Check permissions */
        if (pg_proc_aclcheck(funcOid, GetUserId(), ACL_EXECUTE) == ACLCHECK_NO_PRIV) {
            hash_search(*cache, &funcOid, HASH_REMOVE, &found);
            ereport(ERROR,
                (errmsg("no privilege for \"%s\" to call \"%s\"",
                    format_procedure(fcinfo->flinfo->fn_oid),
                    format_procedure(funcOid))));
        }
    }
    
#if PG_VERSION_NUM >= 90100
    /* Collation support has been added to PostgreSQL with commit
     * d64713df by Tom Lane <tgl@sss.pgh.pa.us>
     * on Tue Apr 12 2011 23:19:24 UTC. First release: PG9.1. */
    InitFunctionCallInfoData(
        funcPtrCallInfo, 
        &cachedFn->info, /* FmgrInfo *flinfo -- ptr to lookup info used for this call */
        PG_NARGS() - 1, /* short nargs -- # arguments actually passed */
        PG_GET_COLLATION(), /* Oid fncollation -- collation for function to use */
        fcinfo->context, /* fmNodePtr context -- pass info about context of call */
        fcinfo->resultinfo /* fmNodePtr resultinfo -- pass or return extra info about result */
    );
#else
    /* an exact copy of the above, but the PG_GET_COLLATION() line removed */
    InitFunctionCallInfoData(
        funcPtrCallInfo, 
        &cachedFn->info, /* FmgrInfo *flinfo -- ptr to lookup info used for this call */
        PG_NARGS() - 1, /* short nargs -- # arguments actually passed */
        fcinfo->context, /* fmNodePtr context -- pass info about context of call */
        fcinfo->resultinfo /* fmNodePtr resultinfo -- pass or return extra info about result */
    );
#endif
    memcpy(funcPtrCallInfo.arg, &fcinfo->arg[1], sizeof(Datum) * (PG_NARGS() - 1));
    memcpy(funcPtrCallInfo.argnull, &fcinfo->argnull[1], sizeof(bool) * (PG_NARGS() - 1));

    HeapTuple procedureTuple = SearchSysCache(PROCOID,
        ObjectIdGetDatum(funcOid), 0, 0, 0);
	if (!HeapTupleIsValid(procedureTuple))
        ereport(ERROR,
            (errmsg("cache lookup failed for function %u", funcOid)));
	Form_pg_proc procedureStruct = (Form_pg_proc) GETSTRUCT(procedureTuple);
    
    for (int i = 1; i < PG_NARGS(); i++)
        if (get_fn_expr_argtype(fcinfo->flinfo, i)
            != procedureStruct->proargtypes.values[i - 1])
            ereport(ERROR,
               (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                errmsg("argument types of \"%s\" and \"%s\" do not match",
                    format_procedure(fcinfo->flinfo->fn_oid),
                    format_procedure(funcOid))));
    
    ReleaseSysCache(procedureTuple);
    
    Datum result = FunctionCallInvoke(&funcPtrCallInfo);
    if (funcPtrCallInfo.isnull)
        PG_RETURN_NULL();
    return result;
}
