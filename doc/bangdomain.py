# -*- coding: utf-8 -*-

import re
import string

from docutils import nodes

from sphinx import addnodes
from sphinx.roles import XRefRole
from sphinx.locale import l_, _
from sphinx.domains import Domain, ObjType
from sphinx.directives import ObjectDescription
from sphinx.util.nodes import make_refnode
from sphinx.util.docfields import Field, TypedField

import bangparser as parser

domain_name = 'bang'
ref_prefix = 'bang.'
xref_prefix = 'bang:'

# RE to split at word boundaries
wsplit_re = re.compile(r'(\W+)')

# REs for C signatures
c_sig_re = re.compile(
    r'''^([^(]*?)          # return type
        ([\w:.]+)  \s*     # thing name (colon allowed for C++)
        (?: \((.*)\) )?    # optionally arguments
        (\s+const)? $      # const specifier
    ''', re.VERBOSE)
c_funcptr_sig_re = re.compile(
    r'''^([^(]+?)          # return type
        (\( [^()]+ \)) \s* # name in parentheses
        \( (.*) \)         # arguments
        (\s+const)? $      # const specifier
    ''', re.VERBOSE)
c_funcptr_arg_sig_re = re.compile(
    r'''^\s*([^(,]+?)      # return type
        \( ([^()]+) \) \s* # name in parentheses
        \( (.*) \)         # arguments
        (\s+const)?        # const specifier
        \s*(?=$|,)         # end with comma or end of string
    ''', re.VERBOSE)
c_funcptr_name_re = re.compile(r'^\(\s*\*\s*(.*?)\s*\)$')

class BangObject(ObjectDescription):
    """
    Description of a Bang language object.
    """

    doc_field_types = [
        TypedField('parameter', label=l_('Parameters'),
                   names=('param', 'parameter', 'arg', 'argument'),
                   typerolename='type', typenames=('type',)),
        Field('returnvalue', label=l_('Returns'), has_arg=False,
              names=('returns', 'return')),
        Field('returntype', label=l_('Return type'), has_arg=False,
              names=('rtype',)),
        Field('prec', label=l_('Precedence'), has_arg=False,
              names=('prec',)),
        Field('assoc', label=l_('Associativity'), has_arg=False,
              names=('assoc',)),
    ]

    # These Bang types aren't described anywhere, so don't try to create
    # a cross-reference to them
    stopwords = set((
        # 'const', 'void', 'char', 'wchar_t', 'int', 'short',
        # 'long', 'float', 'double', 'unsigned', 'signed', 'FILE',
        # 'clock_t', 'time_t', 'ptrdiff_t', 'size_t', 'ssize_t',
        # 'struct', '_Bool',
    ))

    def handle_signature(self, sig, signode):
        """Transform a Bang declaration into RST nodes."""
        # first try the function pointer signature regex, it's more specific
        expr = parser.compile(sig, "<signature>").strip()

        def parse_arg(plist, e):
            type_ = parser.ast_type(e)
            if type_ == "list":
                head = e and e[0]
                mode = None
                if head == "?":
                    paramlist = addnodes.desc_optional()
                    paramlist.child_text_separator = ' '
                    for arg in e[1:]:
                        parse_arg(paramlist, arg)
                    plist += paramlist
                elif head == "|":
                    for i,arg in enumerate(e[1:]):
                        if i > 0:
                            plist += addnodes.desc_parameter("|", " | ", noemph=True)
                        parse_arg(plist, arg)
                else:
                    paramlist = addnodes.desc_parameterlist()
                    paramlist.child_text_separator = ' '
                    for arg in e:
                        parse_arg(paramlist, arg)
                    plist += paramlist
            elif type_ == "symbol":
                if e.startswith('_:'):
                    e = e[2:]
                    plist += addnodes.desc_name(e, " " + e + " ")
                else:
                    plist += addnodes.desc_parameter(e, " " + e + " ")
            else:
                e = repr(e)
                plist += addnodes.desc_parameter(e, e)

        signode += addnodes.desc_annotation(self.objtype, self.objtype + ' ')

        paramlist = addnodes.desc_parameterlist()
        paramlist.child_text_separator = ' '
        if self.objtype in ('infix-macro','ir-infix-macro'):
            name = expr[1]
            parse_arg(paramlist, expr[0])
            paramlist += addnodes.desc_name(name, ' ' + name)
            parse_arg(paramlist, expr[2])
        elif self.objtype in ('symbol-prefix','ir-symbol-prefix'):
            name = expr[0]
            rest = expr[1]
            signode += addnodes.desc_name(name, name)
            paramlist = addnodes.desc_parameter(rest, rest)
        elif self.objtype in ('var','ir-type'):
            name = expr
            paramlist = addnodes.desc_name(name, name)
        else:
            name = expr[0]
            paramlist += addnodes.desc_name(name, ' ' + name + ' ')
            for arg in expr[1:]:
                parse_arg(paramlist, arg)
        signode += paramlist

        return name, self.objtype

    def get_index_text(self, name):
        return '%s (%s)' % name

    def add_target_and_index(self, name, sig, signode):
        #print("add_target_and_index",name,sig,signode)
        # for Bang items we add a prefix since names are usually not qualified
        # by a module name and so easily clash with e.g. section titles
        #name, objtype = name
        keyname = name[1] + '.' + name[0]
        targetname = ref_prefix + keyname
        if targetname not in self.state.document.ids:
            signode['names'].append(targetname)
            signode['ids'].append(targetname)
            signode['first'] = (not self.names)
            self.state.document.note_explicit_target(signode)
            inv = self.env.domaindata[domain_name]['objects']
            if keyname in inv:
                self.state_machine.reporter.warning(
                    'duplicate Bang object description of %s, ' % keyname +
                    'other instance in ' + self.env.doc2path(inv[keyname][0]),
                    line=self.lineno)
            inv[keyname] = (self.env.docname, self.objtype)

        indextext = self.get_index_text(name)
        if indextext:
            self.indexnode['entries'].append(('single', indextext,
                                              targetname, ''))

    def before_content(self):
        #print("before_content")
        self.typename_set = False
        if self.name == 'c:type':
            if self.names:
                self.env.ref_context['c:type'] = self.names[0]
                self.typename_set = True

    def after_content(self):
        #print("after_content")
        if self.typename_set:
            self.env.ref_context.pop('c:type', None)

class BangXRefRole(XRefRole):
    def process_link(self, env, refnode, has_explicit_title, title, target):
        #print("process_link",env,refnode,has_explicit_title,title,target)
        return title, target

class BangDomain(Domain):
    """Bang language domain."""
    name = domain_name
    label = 'Bang'
    object_types = {
        'special': ObjType(l_('special'), 'special'),
        'macro':    ObjType(l_('macro'),    'macro'),
        'infix-macro':    ObjType(l_('infix-macro'), 'infix-macro'),
        'symbol-prefix':    ObjType(l_('symbol-prefix'), 'symbol-prefix'),
        'function': ObjType(l_('function'), 'function'),
        'var': ObjType(l_('var'), 'var'),
        'ir-special': ObjType(l_('ir-special'), 'ir-special'),
        'ir-macro':    ObjType(l_('ir-macro'),    'ir-macro'),
        'ir-infix-macro':    ObjType(l_('ir-infix-macro'), 'ir-infix-macro'),
        'ir-symbol-prefix':    ObjType(l_('ir-symbol-prefix'), 'ir-symbol-prefix'),
        'ir-function': ObjType(l_('ir-function'), 'ir-function'),
        'ir-type': ObjType(l_('ir-type'), 'ir-type'),
    }

    directives = {
        'special': BangObject,
        'macro':    BangObject,
        'infix-macro':    BangObject,
        'symbol-prefix':    BangObject,
        'function': BangObject,
        'var': BangObject,

        'ir-special': BangObject,
        'ir-macro': BangObject,
        'ir-infix-macro': BangObject,
        'ir-symbol-prefix': BangObject,
        'ir-function': BangObject,
        'ir-type': BangObject,
    }
    roles = {
        'func' :  BangXRefRole(),
        'special' :  BangXRefRole(),
        'macro' :  BangXRefRole(),
        'infix-macro' :  BangXRefRole(),
        'symbol-prefix':    BangXRefRole(),
        'var':    BangXRefRole(),
        'obj':    BangXRefRole(),

        'ir-special': BangXRefRole(),
        'ir-macro': BangXRefRole(),
        'ir-infix-macro': BangXRefRole(),
        'ir-symbol-prefix': BangXRefRole(),
        'ir-function': BangXRefRole(),
        'ir-type': BangXRefRole(),
    }

    role_map = {
        'func' : 'function',
    }

    search_roles = [
        'var',
        'macro',
        'infix-macro',
        'function',
        'special',
    ]

    initial_data = {
        'objects': {},  # fullname -> docname, objtype
    }

    def clear_doc(self, docname):
        for fullname, (fn, _) in list(self.data['objects'].items()):
            if fn == docname:
                del self.data['objects'][fullname]

    def merge_domaindata(self, docnames, otherdata):
        # XXX check duplicates
        for fullname, (fn, objtype) in otherdata['objects'].items():
            if fn in docnames:
                self.data['objects'][fullname] = (fn, objtype)

    def resolve_xref(self, env, fromdocname, builder,
                     typ, target, node, contnode):
        if typ == 'obj':
            result = self.resolve_any_xref(env, fromdocname, builder, target, node, contnode)
            if result:
                return result[0][1]
        else:
            typ = self.role_map.get(typ, typ)
            target = typ + '.' + target
            if target not in self.data['objects']:
                return None
            obj = self.data['objects'][target]
            return make_refnode(builder, fromdocname, obj[0], ref_prefix + target,
                                contnode, target)

    def resolve_any_xref(self, env, fromdocname, builder, anytarget,
                         node, contnode):
        result = []
        for role in self.search_roles:
            target = role + '.' + anytarget
            if target not in self.data['objects']:
                continue
            obj = self.data['objects'][target]
            result.append(
                (xref_prefix + self.role_for_objtype(obj[1]),
                     make_refnode(builder, fromdocname, obj[0], ref_prefix + target,
                                  contnode, target)))
        return result

    def get_objects(self):
        for refname, (docname, type) in list(self.data['objects'].items()):
            yield (refname, refname, type, docname, ref_prefix + refname, 1)

def setup(app):
    app.add_domain(BangDomain)