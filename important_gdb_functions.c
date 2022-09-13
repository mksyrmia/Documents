/* The engine for the expression evaluator.  Using the context in this
   object, evaluate the expression between OP_PTR and OP_END.  

   Called by evaluate and dwarf_expr_context::eval functions.
   Has roughly 900 lines of code.    */ 
void
dwarf_expr_context::execute_stack_op (const gdb_byte *op_ptr,
				      const gdb_byte *op_end)


/* Evaluate a location description, starting at DATA and with length
   SIZE, to find the current location of variable of TYPE in the
   context of FRAME.  If SUBOBJ_TYPE is non-NULL, return instead the
   location of the subobject of type SUBOBJ_TYPE at byte offset
   SUBOBJ_BYTE_OFFSET within the variable of type TYPE.  
    
   Called by function dwarf2_evaluate_loc_desc   */
static struct value *
dwarf2_evaluate_loc_desc_full (struct type *type, struct frame_info *frame,
			       const gdb_byte *data, size_t size,
			       dwarf2_per_cu_data *per_cu,
			       dwarf2_per_objfile *per_objfile,
			       struct type *subobj_type,
			       LONGEST subobj_byte_offset,
			       bool as_lval)


/* Fetch the result of the expression evaluation in a form of
   a struct value, where TYPE, SUBOBJ_TYPE and SUBOBJ_OFFSET
   describe the source level representation of that result.
   AS_LVAL defines if the fetched struct value is expected to
   be a value or a location description.  */
value *fetch_result (struct type *type, struct type *subobj_type,
		             LONGEST subobj_offset, bool as_lval);


/* Compute the DWARF CFA (Canonical Frame Address) for a frame.  */
CORE_ADDR dwarf2_frame_cfa (struct frame_info *this_frame);


/* Return a "struct frame_info" corresponding to the frame that called
   THIS_FRAME.  Returns NULL if there is no such frame.

   Unlike get_prev_frame, this function always tries to unwind the
   frame.  */
extern struct frame_info *get_prev_frame_always (struct frame_info *);


/* Return the per-frame unique identifer.  Can be used to relocate a
   frame after a frame cache flush (and other similar operations).  If
   FI is NULL, return the null_frame_id.

   NOTE: kettenis/20040508: These functions return a structure.  On
   platforms where structures are returned in static storage (vax,
   m68k), this may trigger compiler bugs in code like:

   if (frame_id_eq (get_frame_id (l), get_frame_id (r)))

   where the return value from the first get_frame_id (l) gets
   overwritten by the second get_frame_id (r).  Please avoid writing
   code like this.  Use code like:

   struct frame_id id = get_frame_id (l);
   if (frame_id_eq (id, get_frame_id (r)))

   instead, since that avoids the bug.  */
extern struct frame_id get_frame_id (struct frame_info *fi);


/* Compute the frame's uniq ID that can be used to, later, re-find the
   frame.  */
static void
compute_frame_id (struct frame_info *fi)

