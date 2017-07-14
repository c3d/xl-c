#ifndef RENDERER_H
#define RENDERER_H
// ****************************************************************************
//  renderer.h                                      XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Rendering XL parse trees with configurable formatting
//
//
//
//
//
//
//
//
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "tree.h"
#include "syntax.h"


typedef struct renderer *renderer_p;
extern renderer_p       renderer_new(void);
extern void             renderer_delete(renderer_p renderer);

extern syntax_p         renderer_syntax(renderer_p, syntax_p);
extern tree_io_fn       renderer_output_function(renderer_p, tree_io_fn);
extern void *           renderer_output_stream(renderer_p, void *);
extern void             renderer_style(renderer_p, const char *style);
extern void             renderer_reset(renderer_p renderer);

extern void             render(renderer_p, tree_p);
extern void             render_file(renderer_p, tree_p);
extern void             render_text(renderer_p, size_t len, const char *data);
extern void             render_open_quote(renderer_p, char quote);
extern void             render_close_quote(renderer_p, char quote);

#endif // RENDERER_H
