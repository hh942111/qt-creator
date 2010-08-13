/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/
// Copyright (c) 2008 Roberto Raggi <roberto.raggi@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Bind.h"
#include "AST.h"
#include "TranslationUnit.h"
#include "Control.h"
#include "Names.h"
#include "Symbols.h"
#include "CoreTypes.h"
#include "Literals.h"
#include "Scope.h"
#include <vector>
#include <string>
#include <memory>
#include <cassert>

using namespace CPlusPlus;

Bind::Bind(TranslationUnit *unit)
    : ASTVisitor(unit),
      _scope(0),
      _expression(0),
      _name(0),
      _declaratorId(0),
      _visibility(Symbol::Public),
      _methodKey(Function::NormalMethod),
      _skipFunctionBodies(false)
{
}

bool Bind::skipFunctionBodies() const
{
    return _skipFunctionBodies;
}

void Bind::setSkipFunctionBodies(bool skipFunctionBodies)
{
    _skipFunctionBodies = skipFunctionBodies;
}

Scope *Bind::switchScope(Scope *scope)
{
    if (! scope)
        return _scope;

    std::swap(_scope, scope);
    return scope;
}

int Bind::switchVisibility(int visibility)
{
    std::swap(_visibility, visibility);
    return visibility;
}

int Bind::switchMethodKey(int methodKey)
{
    std::swap(_methodKey, methodKey);
    return methodKey;
}

void Bind::operator()(TranslationUnitAST *ast, Namespace *globalNamespace)
{
    Scope *previousScope = switchScope(globalNamespace);
    translationUnit(ast);
    (void) switchScope(previousScope);
}

void Bind::statement(StatementAST *ast)
{
    accept(ast);
}

Bind::ExpressionTy Bind::expression(ExpressionAST *ast)
{
    ExpressionTy value = ExpressionTy();
    std::swap(_expression, value);
    accept(ast);
    std::swap(_expression, value);
    return value;
}

void Bind::declaration(DeclarationAST *ast)
{
    accept(ast);
}

const Name *Bind::name(NameAST *ast)
{
    const Name *value = 0;
    std::swap(_name, value);
    accept(ast);
    std::swap(_name, value);
    return value;
}

FullySpecifiedType Bind::specifier(SpecifierAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType value = init;
    std::swap(_type, value);
    accept(ast);
    std::swap(_type, value);
    return value;
}

FullySpecifiedType Bind::ptrOperator(PtrOperatorAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType value = init;
    std::swap(_type, value);
    accept(ast);
    std::swap(_type, value);
    return value;
}

FullySpecifiedType Bind::coreDeclarator(CoreDeclaratorAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType value = init;
    std::swap(_type, value);
    accept(ast);
    std::swap(_type, value);
    return value;
}

FullySpecifiedType Bind::postfixDeclarator(PostfixDeclaratorAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType value = init;
    std::swap(_type, value);
    accept(ast);
    std::swap(_type, value);
    return value;
}

// AST
bool Bind::visit(ObjCSelectorArgumentAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

const Name *Bind::objCSelectorArgument(ObjCSelectorArgumentAST *ast, bool *hasArg)
{
    if (! (ast && ast->name_token))
        return 0;

    if (ast->colon_token)
        *hasArg = true;

    return control()->nameId(identifier(ast->name_token));
}

bool Bind::visit(AttributeAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::attribute(AttributeAST *ast)
{
    if (! ast)
        return;

    // unsigned identifier_token = ast->identifier_token;
    if (const Identifier *id = identifier(ast->identifier_token)) {
        if (id == control()->deprecatedId())
            _type.setDeprecated(true);
        else if (id == control()->unavailableId())
            _type.setUnavailable(true);
    }

    // unsigned lparen_token = ast->lparen_token;
    // unsigned tag_token = ast->tag_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
}

bool Bind::visit(DeclaratorAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

FullySpecifiedType Bind::declarator(DeclaratorAST *ast, const FullySpecifiedType &init, DeclaratorIdAST **declaratorId)
{
    FullySpecifiedType type = init;

    if (! ast)
        return type;

    std::swap(_declaratorId, declaratorId);
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    for (PtrOperatorListAST *it = ast->ptr_operator_list; it; it = it->next) {
        type = this->ptrOperator(it->value, type);
    }
    for (PostfixDeclaratorListAST *it = ast->postfix_declarator_list; it; it = it->next) {
        type = this->postfixDeclarator(it->value, type);
    }
    type = this->coreDeclarator(ast->core_declarator, type);
    for (SpecifierListAST *it = ast->post_attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned equals_token = ast->equals_token;
    ExpressionTy initializer = this->expression(ast->initializer);
    std::swap(_declaratorId, declaratorId);
    return type;
}

bool Bind::visit(QtPropertyDeclarationItemAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::qtPropertyDeclarationItem(QtPropertyDeclarationItemAST *ast)
{
    if (! ast)
        return;

    // unsigned item_name_token = ast->item_name_token;
    ExpressionTy expression = this->expression(ast->expression);
}

bool Bind::visit(QtInterfaceNameAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::qtInterfaceName(QtInterfaceNameAST *ast)
{
    if (! ast)
        return;

    /*const Name *interface_name =*/ this->name(ast->interface_name);
    for (NameListAST *it = ast->constraint_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
}

bool Bind::visit(BaseSpecifierAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::baseSpecifier(BaseSpecifierAST *ast, unsigned colon_token, Class *klass)
{
    if (! ast)
        return;

    unsigned sourceLocation = ast->firstToken();
    if (! sourceLocation)
        sourceLocation = std::max(colon_token, klass->sourceLocation());

    const Name *baseClassName = this->name(ast->name);
    BaseClass *baseClass = control()->newBaseClass(sourceLocation, baseClassName);
    if (ast->virtual_token)
        baseClass->setVirtual(true);
    if (ast->access_specifier_token) {
        const int visibility = visibilityForAccessSpecifier(tokenKind(ast->access_specifier_token));
        baseClass->setVisibility(visibility); // ### well, not exactly.
    }
    klass->addBaseClass(baseClass);
    ast->symbol = baseClass;
}

bool Bind::visit(CtorInitializerAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::ctorInitializer(CtorInitializerAST *ast, Function *fun)
{
    if (! ast)
        return;

    // unsigned colon_token = ast->colon_token;
    for (MemInitializerListAST *it = ast->member_initializer_list; it; it = it->next) {
        this->memInitializer(it->value, fun);
    }
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
}

bool Bind::visit(EnumeratorAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::enumerator(EnumeratorAST *ast)
{
    if (! ast)
        return;

    // unsigned identifier_token = ast->identifier_token;
    // unsigned equal_token = ast->equal_token;
    ExpressionTy expression = this->expression(ast->expression);
}

bool Bind::visit(ExceptionSpecificationAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

FullySpecifiedType Bind::exceptionSpecification(ExceptionSpecificationAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType type = init;

    if (! ast)
        return type;

    // unsigned throw_token = ast->throw_token;
    // unsigned lparen_token = ast->lparen_token;
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    for (ExpressionListAST *it = ast->type_id_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return type;
}

bool Bind::visit(MemInitializerAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::memInitializer(MemInitializerAST *ast, Function *fun)
{
    if (! ast)
        return;

    /*const Name *name =*/ this->name(ast->name);

    Scope *previousScope = switchScope(fun);
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        /*ExpressionTy value =*/ this->expression(it->value);
    }
    (void) switchScope(previousScope);
}

bool Bind::visit(NestedNameSpecifierAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

const Name *Bind::nestedNameSpecifier(NestedNameSpecifierAST *ast)
{
    if (! ast)
        return 0;

    const Name *class_or_namespace_name = this->name(ast->class_or_namespace_name);
    return class_or_namespace_name;
}

bool Bind::visit(NewPlacementAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::newPlacement(NewPlacementAST *ast)
{
    if (! ast)
        return;

    // unsigned lparen_token = ast->lparen_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
}

bool Bind::visit(NewArrayDeclaratorAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

FullySpecifiedType Bind::newArrayDeclarator(NewArrayDeclaratorAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType type = init;

    if (! ast)
        return type;

    // unsigned lbracket_token = ast->lbracket_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rbracket_token = ast->rbracket_token;
    return type;
}

bool Bind::visit(NewInitializerAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::newInitializer(NewInitializerAST *ast)
{
    if (! ast)
        return;

    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
}

bool Bind::visit(NewTypeIdAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

FullySpecifiedType Bind::newTypeId(NewTypeIdAST *ast)
{
    FullySpecifiedType type;

    if (! ast)
        return type;


    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    for (PtrOperatorListAST *it = ast->ptr_operator_list; it; it = it->next) {
        type = this->ptrOperator(it->value, type);
    }
    for (NewArrayDeclaratorListAST *it = ast->new_array_declarator_list; it; it = it->next) {
        type = this->newArrayDeclarator(it->value, type);
    }
    return type;
}

bool Bind::visit(OperatorAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

int Bind::cppOperator(OperatorAST *ast)
{
    OperatorNameId::Kind kind = OperatorNameId::InvalidOp;

    if (! ast)
        return kind;

    // unsigned op_token = ast->op_token;
    // unsigned open_token = ast->open_token;
    // unsigned close_token = ast->close_token;

    switch (tokenKind(ast->op_token)) {
    case T_NEW:
        if (ast->open_token)
            kind = OperatorNameId::NewArrayOp;
        else
            kind = OperatorNameId::NewOp;
        break;

    case T_DELETE:
        if (ast->open_token)
            kind = OperatorNameId::DeleteArrayOp;
        else
            kind = OperatorNameId::DeleteOp;
        break;

    case T_PLUS:
        kind = OperatorNameId::PlusOp;
        break;

    case T_MINUS:
        kind = OperatorNameId::MinusOp;
        break;

    case T_STAR:
        kind = OperatorNameId::StarOp;
        break;

    case T_SLASH:
        kind = OperatorNameId::SlashOp;
        break;

    case T_PERCENT:
        kind = OperatorNameId::PercentOp;
        break;

    case T_CARET:
        kind = OperatorNameId::CaretOp;
        break;

    case T_AMPER:
        kind = OperatorNameId::AmpOp;
        break;

    case T_PIPE:
        kind = OperatorNameId::PipeOp;
        break;

    case T_TILDE:
        kind = OperatorNameId::TildeOp;
        break;

    case T_EXCLAIM:
        kind = OperatorNameId::ExclaimOp;
        break;

    case T_EQUAL:
        kind = OperatorNameId::EqualOp;
        break;

    case T_LESS:
        kind = OperatorNameId::LessOp;
        break;

    case T_GREATER:
        kind = OperatorNameId::GreaterOp;
        break;

    case T_PLUS_EQUAL:
        kind = OperatorNameId::PlusEqualOp;
        break;

    case T_MINUS_EQUAL:
        kind = OperatorNameId::MinusEqualOp;
        break;

    case T_STAR_EQUAL:
        kind = OperatorNameId::StarEqualOp;
        break;

    case T_SLASH_EQUAL:
        kind = OperatorNameId::SlashEqualOp;
        break;

    case T_PERCENT_EQUAL:
        kind = OperatorNameId::PercentEqualOp;
        break;

    case T_CARET_EQUAL:
        kind = OperatorNameId::CaretEqualOp;
        break;

    case T_AMPER_EQUAL:
        kind = OperatorNameId::AmpEqualOp;
        break;

    case T_PIPE_EQUAL:
        kind = OperatorNameId::PipeEqualOp;
        break;

    case T_LESS_LESS:
        kind = OperatorNameId::LessLessOp;
        break;

    case T_GREATER_GREATER:
        kind = OperatorNameId::GreaterGreaterOp;
        break;

    case T_LESS_LESS_EQUAL:
        kind = OperatorNameId::LessLessEqualOp;
        break;

    case T_GREATER_GREATER_EQUAL:
        kind = OperatorNameId::GreaterGreaterEqualOp;
        break;

    case T_EQUAL_EQUAL:
        kind = OperatorNameId::EqualEqualOp;
        break;

    case T_EXCLAIM_EQUAL:
        kind = OperatorNameId::ExclaimEqualOp;
        break;

    case T_LESS_EQUAL:
        kind = OperatorNameId::LessEqualOp;
        break;

    case T_GREATER_EQUAL:
        kind = OperatorNameId::GreaterEqualOp;
        break;

    case T_AMPER_AMPER:
        kind = OperatorNameId::AmpAmpOp;
        break;

    case T_PIPE_PIPE:
        kind = OperatorNameId::PipePipeOp;
        break;

    case T_PLUS_PLUS:
        kind = OperatorNameId::PlusPlusOp;
        break;

    case T_MINUS_MINUS:
        kind = OperatorNameId::MinusMinusOp;
        break;

    case T_COMMA:
        kind = OperatorNameId::CommaOp;
        break;

    case T_ARROW_STAR:
        kind = OperatorNameId::ArrowStarOp;
        break;

    case T_ARROW:
        kind = OperatorNameId::ArrowOp;
        break;

    case T_LPAREN:
        kind = OperatorNameId::FunctionCallOp;
        break;

    case T_LBRACKET:
        kind = OperatorNameId::ArrayAccessOp;
        break;

    default:
        kind = OperatorNameId::InvalidOp;
    } // switch

    return kind;
}

bool Bind::visit(ParameterDeclarationClauseAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::parameterDeclarationClause(ParameterDeclarationClauseAST *ast, unsigned lparen_token, Function *fun)
{
    if (! ast)
        return;

    if (! fun) {
        translationUnit()->warning(lparen_token, "undefined function");
        return;
    }

    Scope *previousScope = switchScope(fun);

    for (DeclarationListAST *it = ast->parameter_declaration_list; it; it = it->next) {
        this->declaration(it->value);
    }

    if (ast->dot_dot_dot_token)
        fun->setVariadic(true);

    (void) switchScope(previousScope);
}

bool Bind::visit(TranslationUnitAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::translationUnit(TranslationUnitAST *ast)
{
    if (! ast)
        return;

    for (DeclarationListAST *it = ast->declaration_list; it; it = it->next) {
        this->declaration(it->value);
    }
}

bool Bind::visit(ObjCProtocolRefsAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCProtocolRefs(ObjCProtocolRefsAST *ast)
{
    if (! ast)
        return;

    // unsigned less_token = ast->less_token;
    for (NameListAST *it = ast->identifier_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned greater_token = ast->greater_token;
}

bool Bind::visit(ObjCMessageArgumentAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCMessageArgument(ObjCMessageArgumentAST *ast)
{
    if (! ast)
        return;

    ExpressionTy parameter_value_expression = this->expression(ast->parameter_value_expression);
}

bool Bind::visit(ObjCTypeNameAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCTypeName(ObjCTypeNameAST *ast)
{
    if (! ast)
        return;

    // unsigned lparen_token = ast->lparen_token;
    // unsigned type_qualifier_token = ast->type_qualifier_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned rparen_token = ast->rparen_token;
}

bool Bind::visit(ObjCInstanceVariablesDeclarationAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCInstanceVariablesDeclaration(ObjCInstanceVariablesDeclarationAST *ast)
{
    if (! ast)
        return;

    // unsigned lbrace_token = ast->lbrace_token;
    for (DeclarationListAST *it = ast->instance_variable_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned rbrace_token = ast->rbrace_token;
}

bool Bind::visit(ObjCPropertyAttributeAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCPropertyAttribute(ObjCPropertyAttributeAST *ast)
{
    if (! ast)
        return;

    // unsigned attribute_identifier_token = ast->attribute_identifier_token;
    // unsigned equals_token = ast->equals_token;
    /*const Name *method_selector =*/ this->name(ast->method_selector);
}

bool Bind::visit(ObjCMessageArgumentDeclarationAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCMessageArgumentDeclaration(ObjCMessageArgumentDeclarationAST *ast)
{
    if (! ast)
        return;

    this->objCTypeName(ast->type_name);
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    /*const Name *param_name =*/ this->name(ast->param_name);
    // Argument *argument = ast->argument;
}

bool Bind::visit(ObjCMethodPrototypeAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCMethodPrototype(ObjCMethodPrototypeAST *ast)
{
    if (! ast)
        return;

    // unsigned method_type_token = ast->method_type_token;
    this->objCTypeName(ast->type_name);
    /*const Name *selector =*/ this->name(ast->selector);
    for (ObjCMessageArgumentDeclarationListAST *it = ast->argument_list; it; it = it->next) {
        this->objCMessageArgumentDeclaration(it->value);
    }
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // ObjCMethod *symbol = ast->symbol;
}

bool Bind::visit(ObjCSynthesizedPropertyAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::objCSynthesizedProperty(ObjCSynthesizedPropertyAST *ast)
{
    if (! ast)
        return;

    // unsigned property_identifier_token = ast->property_identifier_token;
    // unsigned equals_token = ast->equals_token;
    // unsigned alias_identifier_token = ast->alias_identifier_token;
}

bool Bind::visit(LambdaIntroducerAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::lambdaIntroducer(LambdaIntroducerAST *ast)
{
    if (! ast)
        return;

    // unsigned lbracket_token = ast->lbracket_token;
    this->lambdaCapture(ast->lambda_capture);
    // unsigned rbracket_token = ast->rbracket_token;
}

bool Bind::visit(LambdaCaptureAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::lambdaCapture(LambdaCaptureAST *ast)
{
    if (! ast)
        return;

    // unsigned default_capture_token = ast->default_capture_token;
    for (CaptureListAST *it = ast->capture_list; it; it = it->next) {
        this->capture(it->value);
    }
}

bool Bind::visit(CaptureAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::capture(CaptureAST *ast)
{
    if (! ast)
        return;

}

bool Bind::visit(LambdaDeclaratorAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

void Bind::lambdaDeclarator(LambdaDeclaratorAST *ast)
{
    if (! ast)
        return;


    Function *fun = 0; // ### implement me

    // unsigned lparen_token = ast->lparen_token;
    FullySpecifiedType type;
    this->parameterDeclarationClause(ast->parameter_declaration_clause, ast->lparen_token, fun);
    // unsigned rparen_token = ast->rparen_token;
    for (SpecifierListAST *it = ast->attributes; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned mutable_token = ast->mutable_token;
    type = this->exceptionSpecification(ast->exception_specification, type);
    type = this->trailingReturnType(ast->trailing_return_type, type);
}

bool Bind::visit(TrailingReturnTypeAST *ast)
{
    (void) ast;
    assert(!"unreachable");
    return false;
}

FullySpecifiedType Bind::trailingReturnType(TrailingReturnTypeAST *ast, const FullySpecifiedType &init)
{
    FullySpecifiedType type = init;

    if (! ast)
        return type;

    // unsigned arrow_token = ast->arrow_token;
    for (SpecifierListAST *it = ast->attributes; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    for (SpecifierListAST *it = ast->type_specifiers; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    return type;
}


// StatementAST
bool Bind::visit(QtMemberDeclarationAST *ast)
{
    // unsigned q_token = ast->q_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(CaseStatementAST *ast)
{
    // unsigned case_token = ast->case_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned colon_token = ast->colon_token;
    this->statement(ast->statement);
    return false;
}

bool Bind::visit(CompoundStatementAST *ast)
{
    // unsigned lbrace_token = ast->lbrace_token;
    for (StatementListAST *it = ast->statement_list; it; it = it->next) {
        this->statement(it->value);
    }
    // unsigned rbrace_token = ast->rbrace_token;
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(DeclarationStatementAST *ast)
{
    this->declaration(ast->declaration);
    return false;
}

bool Bind::visit(DoStatementAST *ast)
{
    // unsigned do_token = ast->do_token;
    this->statement(ast->statement);
    // unsigned while_token = ast->while_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ExpressionOrDeclarationStatementAST *ast)
{
    this->statement(ast->expression);
    this->statement(ast->declaration);
    return false;
}

bool Bind::visit(ExpressionStatementAST *ast)
{
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ForeachStatementAST *ast)
{
    // unsigned foreach_token = ast->foreach_token;
    // unsigned lparen_token = ast->lparen_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    ExpressionTy initializer = this->expression(ast->initializer);
    // unsigned comma_token = ast->comma_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ForStatementAST *ast)
{
    // unsigned for_token = ast->for_token;
    // unsigned lparen_token = ast->lparen_token;
    this->statement(ast->initializer);
    ExpressionTy condition = this->expression(ast->condition);
    // unsigned semicolon_token = ast->semicolon_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(IfStatementAST *ast)
{
    // unsigned if_token = ast->if_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy condition = this->expression(ast->condition);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // unsigned else_token = ast->else_token;
    this->statement(ast->else_statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(LabeledStatementAST *ast)
{
    // unsigned label_token = ast->label_token;
    // unsigned colon_token = ast->colon_token;
    this->statement(ast->statement);
    return false;
}

bool Bind::visit(BreakStatementAST *ast)
{
    (void) ast;
    // unsigned break_token = ast->break_token;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ContinueStatementAST *ast)
{
    (void) ast;
    // unsigned continue_token = ast->continue_token;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(GotoStatementAST *ast)
{
    (void) ast;
    // unsigned goto_token = ast->goto_token;
    // unsigned identifier_token = ast->identifier_token;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ReturnStatementAST *ast)
{
    // unsigned return_token = ast->return_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(SwitchStatementAST *ast)
{
    // unsigned switch_token = ast->switch_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy condition = this->expression(ast->condition);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(TryBlockStatementAST *ast)
{
    // unsigned try_token = ast->try_token;
    this->statement(ast->statement);
    for (CatchClauseListAST *it = ast->catch_clause_list; it; it = it->next) {
        this->statement(it->value);
    }
    return false;
}

bool Bind::visit(CatchClauseAST *ast)
{
    // unsigned catch_token = ast->catch_token;
    // unsigned lparen_token = ast->lparen_token;
    this->declaration(ast->exception_declaration);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(WhileStatementAST *ast)
{
    // unsigned while_token = ast->while_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy condition = this->expression(ast->condition);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ObjCFastEnumerationAST *ast)
{
    // unsigned for_token = ast->for_token;
    // unsigned lparen_token = ast->lparen_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    ExpressionTy initializer = this->expression(ast->initializer);
    // unsigned in_token = ast->in_token;
    ExpressionTy fast_enumeratable_expression = this->expression(ast->fast_enumeratable_expression);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    // Block *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ObjCSynchronizedStatementAST *ast)
{
    // unsigned synchronized_token = ast->synchronized_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy synchronized_object = this->expression(ast->synchronized_object);
    // unsigned rparen_token = ast->rparen_token;
    this->statement(ast->statement);
    return false;
}


// ExpressionAST
bool Bind::visit(IdExpressionAST *ast)
{
    /*const Name *name =*/ this->name(ast->name);
    return false;
}

bool Bind::visit(CompoundExpressionAST *ast)
{
    // unsigned lparen_token = ast->lparen_token;
    this->statement(ast->statement);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(CompoundLiteralAST *ast)
{
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned rparen_token = ast->rparen_token;
    ExpressionTy initializer = this->expression(ast->initializer);
    return false;
}

bool Bind::visit(QtMethodAST *ast)
{
    // unsigned method_token = ast->method_token;
    // unsigned lparen_token = ast->lparen_token;
    FullySpecifiedType type;
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(BinaryExpressionAST *ast)
{
    ExpressionTy left_expression = this->expression(ast->left_expression);
    // unsigned binary_op_token = ast->binary_op_token;
    ExpressionTy right_expression = this->expression(ast->right_expression);
    return false;
}

bool Bind::visit(CastExpressionAST *ast)
{
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned rparen_token = ast->rparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    return false;
}

bool Bind::visit(ConditionAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    return false;
}

bool Bind::visit(ConditionalExpressionAST *ast)
{
    ExpressionTy condition = this->expression(ast->condition);
    // unsigned question_token = ast->question_token;
    ExpressionTy left_expression = this->expression(ast->left_expression);
    // unsigned colon_token = ast->colon_token;
    ExpressionTy right_expression = this->expression(ast->right_expression);
    return false;
}

bool Bind::visit(CppCastExpressionAST *ast)
{
    // unsigned cast_token = ast->cast_token;
    // unsigned less_token = ast->less_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned greater_token = ast->greater_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(DeleteExpressionAST *ast)
{
    // unsigned scope_token = ast->scope_token;
    // unsigned delete_token = ast->delete_token;
    // unsigned lbracket_token = ast->lbracket_token;
    // unsigned rbracket_token = ast->rbracket_token;
    ExpressionTy expression = this->expression(ast->expression);
    return false;
}

bool Bind::visit(ArrayInitializerAST *ast)
{
    // unsigned lbrace_token = ast->lbrace_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rbrace_token = ast->rbrace_token;
    return false;
}

bool Bind::visit(NewExpressionAST *ast)
{
    // unsigned scope_token = ast->scope_token;
    // unsigned new_token = ast->new_token;
    this->newPlacement(ast->new_placement);
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // unsigned rparen_token = ast->rparen_token;
    this->newTypeId(ast->new_type_id);
    this->newInitializer(ast->new_initializer);
    return false;
}

bool Bind::visit(TypeidExpressionAST *ast)
{
    // unsigned typeid_token = ast->typeid_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(TypenameCallExpressionAST *ast)
{
    // unsigned typename_token = ast->typename_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned lparen_token = ast->lparen_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(TypeConstructorCallAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned lparen_token = ast->lparen_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(SizeofExpressionAST *ast)
{
    // unsigned sizeof_token = ast->sizeof_token;
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(NumericLiteralAST *ast)
{
    (void) ast;
    // unsigned literal_token = ast->literal_token;
    return false;
}

bool Bind::visit(BoolLiteralAST *ast)
{
    (void) ast;
    // unsigned literal_token = ast->literal_token;
    return false;
}

bool Bind::visit(ThisExpressionAST *ast)
{
    (void) ast;
    // unsigned this_token = ast->this_token;
    return false;
}

bool Bind::visit(NestedExpressionAST *ast)
{
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(StringLiteralAST *ast)
{
    // unsigned literal_token = ast->literal_token;
    ExpressionTy next = this->expression(ast->next);
    return false;
}

bool Bind::visit(ThrowExpressionAST *ast)
{
    // unsigned throw_token = ast->throw_token;
    ExpressionTy expression = this->expression(ast->expression);
    return false;
}

bool Bind::visit(TypeIdAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    return false;
}

bool Bind::visit(UnaryExpressionAST *ast)
{
    // unsigned unary_op_token = ast->unary_op_token;
    ExpressionTy expression = this->expression(ast->expression);
    return false;
}

bool Bind::visit(ObjCMessageExpressionAST *ast)
{
    // unsigned lbracket_token = ast->lbracket_token;
    /*ExpressionTy receiver_expression =*/ this->expression(ast->receiver_expression);
    /*const Name *selector =*/ this->name(ast->selector);
    for (ObjCMessageArgumentListAST *it = ast->argument_list; it; it = it->next) {
        this->objCMessageArgument(it->value);
    }
    // unsigned rbracket_token = ast->rbracket_token;
    return false;
}

bool Bind::visit(ObjCProtocolExpressionAST *ast)
{
    (void) ast;
    // unsigned protocol_token = ast->protocol_token;
    // unsigned lparen_token = ast->lparen_token;
    // unsigned identifier_token = ast->identifier_token;
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(ObjCEncodeExpressionAST *ast)
{
    // unsigned encode_token = ast->encode_token;
    this->objCTypeName(ast->type_name);
    return false;
}

bool Bind::visit(ObjCSelectorExpressionAST *ast)
{
    // unsigned selector_token = ast->selector_token;
    // unsigned lparen_token = ast->lparen_token;
    /*const Name *selector =*/ this->name(ast->selector);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(LambdaExpressionAST *ast)
{
    this->lambdaIntroducer(ast->lambda_introducer);
    this->lambdaDeclarator(ast->lambda_declarator);
    this->statement(ast->statement);
    return false;
}

bool Bind::visit(BracedInitializerAST *ast)
{
    // unsigned lbrace_token = ast->lbrace_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned comma_token = ast->comma_token;
    // unsigned rbrace_token = ast->rbrace_token;
    return false;
}


// DeclarationAST
bool Bind::visit(SimpleDeclarationAST *ast)
{
    // unsigned qt_invokable_token = ast->qt_invokable_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->decl_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }

    List<Declaration *> **symbolTail = &ast->symbols;

    for (DeclaratorListAST *it = ast->declarator_list; it; it = it->next) {
        DeclaratorIdAST *declaratorId = 0;
        FullySpecifiedType declTy = this->declarator(it->value, type.qualifiedType(), &declaratorId);

        const Name *declName = 0;
        unsigned sourceLocation = ast->firstToken();
        if (declaratorId && declaratorId->name) {
            sourceLocation = declaratorId->firstToken();
            declName = declaratorId->name->name;
        }

        Declaration *decl = control()->newDeclaration(sourceLocation, declName);
        decl->setType(declTy);
        if (_scope->isClass()) {
            decl->setVisibility(_visibility);

            if (Function *funTy = decl->type()->asFunctionType()) {
                funTy->setMethodKey(_methodKey);
            }
        }
        _scope->addMember(decl);

        *symbolTail = new (translationUnit()->memoryPool()) List<Declaration *>(decl);
        symbolTail = &(*symbolTail)->next;
    }
    return false;
}

bool Bind::visit(EmptyDeclarationAST *ast)
{
    (void) ast;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(AccessDeclarationAST *ast)
{
    (void) ast;
    // unsigned access_specifier_token = ast->access_specifier_token;
    // unsigned slots_token = ast->slots_token;
    // unsigned colon_token = ast->colon_token;

    const int accessSpecifier = tokenKind(ast->access_specifier_token);
    _visibility = visibilityForAccessSpecifier(accessSpecifier);

    if (ast->slots_token)
        _methodKey = Function::SlotMethod;
    else if (accessSpecifier == T_Q_SIGNALS)
        _methodKey = Function::SignalMethod;
    else
        _methodKey = Function::NormalMethod;

    return false;
}

bool Bind::visit(QtObjectTagAST *ast)
{
    (void) ast;
    // unsigned q_object_token = ast->q_object_token;
    return false;
}

bool Bind::visit(QtPrivateSlotAST *ast)
{
    // unsigned q_private_slot_token = ast->q_private_slot_token;
    // unsigned lparen_token = ast->lparen_token;
    // unsigned dptr_token = ast->dptr_token;
    // unsigned dptr_lparen_token = ast->dptr_lparen_token;
    // unsigned dptr_rparen_token = ast->dptr_rparen_token;
    // unsigned comma_token = ast->comma_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifiers; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(QtPropertyDeclarationAST *ast)
{
    // unsigned property_specifier_token = ast->property_specifier_token;
    // unsigned lparen_token = ast->lparen_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    /*const Name *property_name =*/ this->name(ast->property_name);
    for (QtPropertyDeclarationItemListAST *it = ast->property_declaration_items; it; it = it->next) {
        this->qtPropertyDeclarationItem(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(QtEnumDeclarationAST *ast)
{
    // unsigned enum_specifier_token = ast->enum_specifier_token;
    // unsigned lparen_token = ast->lparen_token;
    for (NameListAST *it = ast->enumerator_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(QtFlagsDeclarationAST *ast)
{
    // unsigned flags_specifier_token = ast->flags_specifier_token;
    // unsigned lparen_token = ast->lparen_token;
    for (NameListAST *it = ast->flag_enums_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(QtInterfacesDeclarationAST *ast)
{
    // unsigned interfaces_token = ast->interfaces_token;
    // unsigned lparen_token = ast->lparen_token;
    for (QtInterfaceNameListAST *it = ast->interface_name_list; it; it = it->next) {
        this->qtInterfaceName(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(AsmDefinitionAST *ast)
{
    (void) ast;
    // unsigned asm_token = ast->asm_token;
    // unsigned volatile_token = ast->volatile_token;
    // unsigned lparen_token = ast->lparen_token;
    // unsigned rparen_token = ast->rparen_token;
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ExceptionDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    return false;
}

bool Bind::visit(FunctionDefinitionAST *ast)
{
    // unsigned qt_invokable_token = ast->qt_invokable_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->decl_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    Function *fun = type->asFunctionType();
    ast->symbol = fun;

    if (fun) {
        if (_scope->isClass()) {
            fun->setVisibility(_visibility);
            fun->setMethodKey(_methodKey);
        }
        _scope->addMember(fun);
    } else
        translationUnit()->warning(ast->firstToken(), "expected a function declarator");

    this->ctorInitializer(ast->ctor_initializer, fun);

    if (! _skipFunctionBodies) {
        Scope *previousScope = switchScope(fun);
        this->statement(ast->function_body);
        (void) switchScope(previousScope);
    }

    // Function *symbol = ast->symbol;
    return false;
}

bool Bind::visit(LinkageBodyAST *ast)
{
    // unsigned lbrace_token = ast->lbrace_token;
    for (DeclarationListAST *it = ast->declaration_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned rbrace_token = ast->rbrace_token;
    return false;
}

bool Bind::visit(LinkageSpecificationAST *ast)
{
    // unsigned extern_token = ast->extern_token;
    // unsigned extern_type_token = ast->extern_type_token;
    this->declaration(ast->declaration);
    return false;
}

bool Bind::visit(NamespaceAST *ast)
{
    // unsigned namespace_token = ast->namespace_token;
    // unsigned identifier_token = ast->identifier_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }

    unsigned sourceLocation = ast->firstToken();
    const Name *namespaceName = 0;
    if (ast->identifier_token) {
        sourceLocation = ast->identifier_token;
        namespaceName = control()->nameId(identifier(ast->identifier_token));
    }

    Namespace *ns = control()->newNamespace(sourceLocation, namespaceName);
    ast->symbol = ns;
    _scope->addMember(ns);

    Scope *previousScope = switchScope(ns);
    this->declaration(ast->linkage_body);
    (void) switchScope(previousScope);
    return false;
}

bool Bind::visit(NamespaceAliasDefinitionAST *ast)
{
    // unsigned namespace_token = ast->namespace_token;
    // unsigned namespace_name_token = ast->namespace_name_token;
    // unsigned equal_token = ast->equal_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ParameterDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    DeclaratorIdAST *declaratorId = 0;
    type = this->declarator(ast->declarator, type, &declaratorId);
    // unsigned equal_token = ast->equal_token;
    ExpressionTy expression = this->expression(ast->expression);

    unsigned sourceLocation = ast->firstToken();
    if (declaratorId)
        sourceLocation = declaratorId->firstToken();

    const Name *argName = 0;
    if (declaratorId && declaratorId->name)
        argName = declaratorId->name->name;

    Argument *arg = control()->newArgument(sourceLocation, argName);
    arg->setType(type);

    if (ast->expression) {
        unsigned startOfExpression = ast->expression->firstToken();
        unsigned endOfExpression = ast->expression->lastToken();
        std::string buffer;
        for (unsigned index = startOfExpression; index != endOfExpression; ++index) {
            const Token &tk = tokenAt(index);
            if (tk.whitespace() || tk.newline())
                buffer += ' ';
            buffer += tk.spell();
        }
        const StringLiteral *initializer = control()->stringLiteral(buffer.c_str(), buffer.size());
        arg->setInitializer(initializer);
    }

    _scope->addMember(arg);

    ast->symbol = arg;
    return false;
}

bool Bind::visit(TemplateDeclarationAST *ast)
{
    // unsigned export_token = ast->export_token;
    // unsigned template_token = ast->template_token;
    // unsigned less_token = ast->less_token;
    for (DeclarationListAST *it = ast->template_parameter_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned greater_token = ast->greater_token;
    this->declaration(ast->declaration);
    // Template *symbol = ast->symbol;
    return false;
}

bool Bind::visit(TypenameTypeParameterAST *ast)
{
    // unsigned classkey_token = ast->classkey_token;
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned equal_token = ast->equal_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // TypenameArgument *symbol = ast->symbol;
    return false;
}

bool Bind::visit(TemplateTypeParameterAST *ast)
{
    // unsigned template_token = ast->template_token;
    // unsigned less_token = ast->less_token;
    for (DeclarationListAST *it = ast->template_parameter_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned greater_token = ast->greater_token;
    // unsigned class_token = ast->class_token;
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned equal_token = ast->equal_token;
    ExpressionTy type_id = this->expression(ast->type_id);
    // TypenameArgument *symbol = ast->symbol;
    return false;
}

bool Bind::visit(UsingAST *ast)
{
    // unsigned using_token = ast->using_token;
    // unsigned typename_token = ast->typename_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned semicolon_token = ast->semicolon_token;
    // UsingDeclaration *symbol = ast->symbol;
    return false;
}

bool Bind::visit(UsingDirectiveAST *ast)
{
    // unsigned using_token = ast->using_token;
    // unsigned namespace_token = ast->namespace_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned semicolon_token = ast->semicolon_token;
    // UsingNamespaceDirective *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ObjCClassForwardDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned class_token = ast->class_token;
    for (NameListAST *it = ast->identifier_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned semicolon_token = ast->semicolon_token;
    // List<ObjCForwardClassDeclaration *> *symbols = ast->symbols;
    return false;
}

bool Bind::visit(ObjCClassDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned interface_token = ast->interface_token;
    // unsigned implementation_token = ast->implementation_token;
    /*const Name *class_name =*/ this->name(ast->class_name);
    // unsigned lparen_token = ast->lparen_token;
    /*const Name *category_name =*/ this->name(ast->category_name);
    // unsigned rparen_token = ast->rparen_token;
    // unsigned colon_token = ast->colon_token;
    /*const Name *superclass =*/ this->name(ast->superclass);
    this->objCProtocolRefs(ast->protocol_refs);
    this->objCInstanceVariablesDeclaration(ast->inst_vars_decl);
    for (DeclarationListAST *it = ast->member_declaration_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned end_token = ast->end_token;
    // ObjCClass *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ObjCProtocolForwardDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned protocol_token = ast->protocol_token;
    for (NameListAST *it = ast->identifier_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned semicolon_token = ast->semicolon_token;
    // List<ObjCForwardProtocolDeclaration *> *symbols = ast->symbols;
    return false;
}

bool Bind::visit(ObjCProtocolDeclarationAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned protocol_token = ast->protocol_token;
    /*const Name *name =*/ this->name(ast->name);
    this->objCProtocolRefs(ast->protocol_refs);
    for (DeclarationListAST *it = ast->member_declaration_list; it; it = it->next) {
        this->declaration(it->value);
    }
    // unsigned end_token = ast->end_token;
    // ObjCProtocol *symbol = ast->symbol;
    return false;
}

bool Bind::visit(ObjCVisibilityDeclarationAST *ast)
{
    (void) ast;
    // unsigned visibility_token = ast->visibility_token;
    return false;
}

bool Bind::visit(ObjCPropertyDeclarationAST *ast)
{
    (void) ast;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    // unsigned property_token = ast->property_token;
    // unsigned lparen_token = ast->lparen_token;
    for (ObjCPropertyAttributeListAST *it = ast->property_attribute_list; it; it = it->next) {
        this->objCPropertyAttribute(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    this->declaration(ast->simple_declaration);
    // List<ObjCPropertyDeclaration *> *symbols = ast->symbols;
    return false;
}

bool Bind::visit(ObjCMethodDeclarationAST *ast)
{
    this->objCMethodPrototype(ast->method_prototype);
    this->statement(ast->function_body);
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ObjCSynthesizedPropertiesDeclarationAST *ast)
{
    // unsigned synthesized_token = ast->synthesized_token;
    for (ObjCSynthesizedPropertyListAST *it = ast->property_identifier_list; it; it = it->next) {
        this->objCSynthesizedProperty(it->value);
    }
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}

bool Bind::visit(ObjCDynamicPropertiesDeclarationAST *ast)
{
    // unsigned dynamic_token = ast->dynamic_token;
    for (NameListAST *it = ast->property_identifier_list; it; it = it->next) {
        /*const Name *value =*/ this->name(it->value);
    }
    // unsigned semicolon_token = ast->semicolon_token;
    return false;
}


// NameAST
bool Bind::visit(ObjCSelectorAST *ast) // ### review
{
    std::vector<const Name *> arguments;
    bool hasArgs = false;

    for (ObjCSelectorArgumentListAST *it = ast->selector_argument_list; it; it = it->next) {
        if (const Name *selector_argument = this->objCSelectorArgument(it->value, &hasArgs))
            arguments.push_back(selector_argument);
    }

    if (! arguments.empty()) {
        _name = control()->selectorNameId(&arguments[0], arguments.size(), hasArgs);
        ast->name = _name;
    }

    return false;
}

bool Bind::visit(QualifiedNameAST *ast)
{
    for (NestedNameSpecifierListAST *it = ast->nested_name_specifier_list; it; it = it->next) {
        const Name *class_or_namespace_name = this->nestedNameSpecifier(it->value);
        if (_name || ast->global_scope_token)
            _name = control()->qualifiedNameId(_name, class_or_namespace_name);
        else
            _name = class_or_namespace_name;
    }

    const Name *unqualified_name = this->name(ast->unqualified_name);
    if (_name || ast->global_scope_token)
        _name = control()->qualifiedNameId(_name, unqualified_name);
    else
        _name = unqualified_name;

    ast->name = _name;
    return false;
}

bool Bind::visit(OperatorFunctionIdAST *ast)
{
    const int op = this->cppOperator(ast->op);
    ast->name = _name = control()->operatorNameId(op);
    return false;
}

bool Bind::visit(ConversionFunctionIdAST *ast)
{
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->type_specifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    for (PtrOperatorListAST *it = ast->ptr_operator_list; it; it = it->next) {
        type = this->ptrOperator(it->value, type);
    }
    ast->name = _name = control()->conversionNameId(type);
    return false;
}

bool Bind::visit(SimpleNameAST *ast)
{
    const Identifier *id = identifier(ast->identifier_token);
    ast->name = _name = control()->nameId(id);
    return false;
}

bool Bind::visit(DestructorNameAST *ast)
{
    const Identifier *id = identifier(ast->identifier_token);
    ast->name = _name = control()->destructorNameId(id);
    return false;
}

bool Bind::visit(TemplateIdAST *ast)
{
    // collect the template parameters
    std::vector<FullySpecifiedType> templateArguments;
    for (ExpressionListAST *it = ast->template_argument_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
        templateArguments.push_back(value);
    }

    const Identifier *id = identifier(ast->identifier_token);
    if (templateArguments.empty())
        _name = control()->templateNameId(id);
    else
        _name = control()->templateNameId(id, &templateArguments[0], templateArguments.size());

    ast->name = _name;
    return false;
}


// SpecifierAST
bool Bind::visit(SimpleSpecifierAST *ast)
{
    switch (tokenKind(ast->specifier_token)) {
        case T_CONST:
            if (_type.isConst())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setConst(true);
            break;

        case T_VOLATILE:
            if (_type.isVolatile())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setVolatile(true);
            break;

        case T_FRIEND:
            if (_type.isFriend())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setFriend(true);
            break;

        case T_AUTO:
            if (_type.isAuto())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setAuto(true);
            break;

        case T_REGISTER:
            if (_type.isRegister())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setRegister(true);
            break;

        case T_STATIC:
            if (_type.isStatic())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setStatic(true);
            break;

        case T_EXTERN:
            if (_type.isExtern())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setExtern(true);
            break;

        case T_MUTABLE:
            if (_type.isMutable())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setMutable(true);
            break;

        case T_TYPEDEF:
            if (_type.isTypedef())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setTypedef(true);
            break;

        case T_INLINE:
            if (_type.isInline())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setInline(true);
            break;

        case T_VIRTUAL:
            if (_type.isVirtual())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setVirtual(true);
            break;

        case T_EXPLICIT:
            if (_type.isExplicit())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setExplicit(true);
            break;

        case T_SIGNED:
            if (_type.isSigned())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setSigned(true);
            break;

        case T_UNSIGNED:
            if (_type.isUnsigned())
                translationUnit()->error(ast->specifier_token, "duplicate `%s'", spell(ast->specifier_token));
            _type.setUnsigned(true);
            break;

        case T_CHAR:
            if (_type)
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            _type.setType(control()->integerType(IntegerType::Char));
            break;

        case T_WCHAR_T:
            if (_type)
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            _type.setType(control()->integerType(IntegerType::WideChar));
            break;

        case T_BOOL:
            if (_type)
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            _type.setType(control()->integerType(IntegerType::Bool));
            break;

        case T_SHORT:
            if (_type) {
                IntegerType *intType = control()->integerType(IntegerType::Int);
                if (_type.type() != intType)
                    translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            }
            _type.setType(control()->integerType(IntegerType::Short));
            break;

        case T_INT:
            if (_type) {
                Type *tp = _type.type();
                IntegerType *shortType = control()->integerType(IntegerType::Short);
                IntegerType *longType = control()->integerType(IntegerType::Long);
                IntegerType *longLongType = control()->integerType(IntegerType::LongLong);
                if (tp == shortType || tp == longType || tp == longLongType)
                    break;
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            }
            _type.setType(control()->integerType(IntegerType::Int));
            break;

        case T_LONG:
            if (_type) {
                Type *tp = _type.type();
                IntegerType *intType = control()->integerType(IntegerType::Int);
                IntegerType *longType = control()->integerType(IntegerType::Long);
                FloatType *doubleType = control()->floatType(FloatType::Double);
                if (tp == longType) {
                    _type.setType(control()->integerType(IntegerType::LongLong));
                    break;
                } else if (tp == doubleType) {
                    _type.setType(control()->floatType(FloatType::LongDouble));
                    break;
                } else if (tp != intType) {
                    translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
                }
            }
            _type.setType(control()->integerType(IntegerType::Long));
            break;

        case T_FLOAT:
            if (_type)
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            _type.setType(control()->floatType(FloatType::Float));
            break;

        case T_DOUBLE:
            if (_type) {
                IntegerType *longType = control()->integerType(IntegerType::Long);
                if (_type.type() == longType) {
                    _type.setType(control()->floatType(FloatType::LongDouble));
                    break;
                }
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            }
            _type.setType(control()->floatType(FloatType::Double));
            break;

        case T_VOID:
            if (_type)
                translationUnit()->error(ast->specifier_token, "duplicate data type in declaration");
            _type.setType(control()->voidType());
            break;

        default:
            break;
    } // switch
    return false;
}

bool Bind::visit(AttributeSpecifierAST *ast)
{
    // unsigned attribute_token = ast->attribute_token;
    // unsigned first_lparen_token = ast->first_lparen_token;
    // unsigned second_lparen_token = ast->second_lparen_token;
    for (AttributeListAST *it = ast->attribute_list; it; it = it->next) {
        this->attribute(it->value);
    }
    // unsigned first_rparen_token = ast->first_rparen_token;
    // unsigned second_rparen_token = ast->second_rparen_token;
    return false;
}

bool Bind::visit(TypeofSpecifierAST *ast)
{
    ExpressionTy expression = this->expression(ast->expression);
    _type = expression;
    return false;
}

bool Bind::visit(ClassSpecifierAST *ast)
{
    // unsigned classkey_token = ast->classkey_token;
    unsigned sourceLocation = ast->classkey_token;

    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        _type = this->specifier(it->value, _type);
    }

    const Name *className = this->name(ast->name);

    if (ast->name) {
        sourceLocation = ast->name->firstToken();

        if (QualifiedNameAST *q = ast->name->asQualifiedName()) {
            if (q->unqualified_name)
                sourceLocation = q->unqualified_name->firstToken();
        }
    }

    Class *klass = control()->newClass(sourceLocation, className);
    _scope->addMember(klass);

    _type.setType(klass);

    Scope *previousScope = switchScope(klass);
    const int previousVisibility = switchVisibility(Symbol::Public);
    const int previousMethodKey = switchMethodKey(Function::NormalMethod);

    for (BaseSpecifierListAST *it = ast->base_clause_list; it; it = it->next) {
        this->baseSpecifier(it->value, ast->colon_token, klass);
    }
    // unsigned dot_dot_dot_token = ast->dot_dot_dot_token;
    for (DeclarationListAST *it = ast->member_specifier_list; it; it = it->next) {
        this->declaration(it->value);
    }

    (void) switchMethodKey(previousMethodKey);
    (void) switchVisibility(previousVisibility);
    (void) switchScope(previousScope);

    ast->symbol = klass;
    return false;
}

bool Bind::visit(NamedTypeSpecifierAST *ast)
{
    /*const Name *name =*/ this->name(ast->name);
    return false;
}

bool Bind::visit(ElaboratedTypeSpecifierAST *ast)
{
    // unsigned classkey_token = ast->classkey_token;
    FullySpecifiedType type;
    for (SpecifierListAST *it = ast->attribute_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    /*const Name *name =*/ this->name(ast->name);
    return false;
}

bool Bind::visit(EnumSpecifierAST *ast)
{
    // unsigned enum_token = ast->enum_token;
    /*const Name *name =*/ this->name(ast->name);
    // unsigned lbrace_token = ast->lbrace_token;
    for (EnumeratorListAST *it = ast->enumerator_list; it; it = it->next) {
        this->enumerator(it->value);
    }
    // unsigned rbrace_token = ast->rbrace_token;
    // Enum *symbol = ast->symbol;
    return false;
}


// PtrOperatorAST
bool Bind::visit(PointerToMemberAST *ast)
{
    const Name *memberName = 0;

    for (NestedNameSpecifierListAST *it = ast->nested_name_specifier_list; it; it = it->next) {
        const Name *class_or_namespace_name = this->nestedNameSpecifier(it->value);
        if (memberName || ast->global_scope_token)
            memberName = control()->qualifiedNameId(memberName, class_or_namespace_name);
        else
            memberName = class_or_namespace_name;
    }

    FullySpecifiedType type(control()->pointerToMemberType(memberName, _type));
    for (SpecifierListAST *it = ast->cv_qualifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    _type = type;
    return false;
}

bool Bind::visit(PointerAST *ast)
{
    if (_type->isReferenceType())
        translationUnit()->error(ast->firstToken(), "cannot declare pointer to a reference");

    FullySpecifiedType type(control()->pointerType(_type));
    for (SpecifierListAST *it = ast->cv_qualifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }
    _type = type;
    return false;
}

bool Bind::visit(ReferenceAST *ast)
{
    const bool rvalueRef = (tokenKind(ast->reference_token) == T_AMPER_AMPER);

    if (_type->isReferenceType())
        translationUnit()->error(ast->firstToken(), "cannot declare reference to a reference");

    FullySpecifiedType type(control()->referenceType(_type, rvalueRef));
    _type = type;
    return false;
}


// PostfixAST
bool Bind::visit(CallAST *ast)
{
    ExpressionTy base_expression = this->expression(ast->base_expression);
    // unsigned lparen_token = ast->lparen_token;
    for (ExpressionListAST *it = ast->expression_list; it; it = it->next) {
        ExpressionTy value = this->expression(it->value);
    }
    // unsigned rparen_token = ast->rparen_token;
    return false;
}

bool Bind::visit(ArrayAccessAST *ast)
{
    ExpressionTy base_expression = this->expression(ast->base_expression);
    // unsigned lbracket_token = ast->lbracket_token;
    ExpressionTy expression = this->expression(ast->expression);
    // unsigned rbracket_token = ast->rbracket_token;
    return false;
}

bool Bind::visit(PostIncrDecrAST *ast)
{
    ExpressionTy base_expression = this->expression(ast->base_expression);
    // unsigned incr_decr_token = ast->incr_decr_token;
    return false;
}

bool Bind::visit(MemberAccessAST *ast)
{
    ExpressionTy base_expression = this->expression(ast->base_expression);
    // unsigned access_token = ast->access_token;
    // unsigned template_token = ast->template_token;
    /*const Name *member_name =*/ this->name(ast->member_name);
    return false;
}


// CoreDeclaratorAST
bool Bind::visit(DeclaratorIdAST *ast)
{
    /*const Name *name =*/ this->name(ast->name);
    *_declaratorId = ast;
    return false;
}

bool Bind::visit(NestedDeclaratorAST *ast)
{
    _type = this->declarator(ast->declarator, _type, _declaratorId);
    return false;
}


// PostfixDeclaratorAST
bool Bind::visit(FunctionDeclaratorAST *ast)
{
    Function *fun = control()->newFunction(0, 0);
    fun->setReturnType(_type);

    // unsigned lparen_token = ast->lparen_token;
    this->parameterDeclarationClause(ast->parameters, ast->lparen_token, fun);
    // unsigned rparen_token = ast->rparen_token;
    FullySpecifiedType type(fun);
    for (SpecifierListAST *it = ast->cv_qualifier_list; it; it = it->next) {
        type = this->specifier(it->value, type);
    }

    // propagate the cv-qualifiers
    fun->setConst(type.isConst());
    fun->setVolatile(type.isVolatile());

    this->exceptionSpecification(ast->exception_specification, type);
    this->trailingReturnType(ast->trailing_return_type, type);
    if (ast->as_cpp_initializer != 0) {
        fun->setAmbiguous(true);
        ExpressionTy as_cpp_initializer = this->expression(ast->as_cpp_initializer);
    }
    ast->symbol = fun;
    _type = type;
    return false;
}

bool Bind::visit(ArrayDeclaratorAST *ast)
{
    ExpressionTy expression = this->expression(ast->expression);
    FullySpecifiedType type(control()->arrayType(_type));
    _type = type;
    return false;
}

int Bind::visibilityForAccessSpecifier(int tokenKind)
{
    switch (tokenKind) {
    case T_PUBLIC:
        return Symbol::Public;
    case T_PROTECTED:
        return Symbol::Protected;
    case T_PRIVATE:
        return Symbol::Private;
    case T_Q_SIGNALS:
        return Symbol::Protected;
    default:
        return Symbol::Public;
    }
}
