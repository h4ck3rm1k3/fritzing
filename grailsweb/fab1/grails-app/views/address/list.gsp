
<%@ page import="com.g2one.fab1.Address" %>
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
        <meta name="layout" content="main" />
        <g:set var="entityName" value="${message(code: 'address.label', default: 'Address')}" />
        <title><g:message code="default.list.label" args="[entityName]" /></title>
    </head>
    <body>
        <div class="nav">
            <span class="menuButton"><a class="home" href="${createLink(uri: '/')}">Home</a></span>
            <span class="menuButton"><g:link class="create" action="create"><g:message code="default.new.label" args="[entityName]" /></g:link></span>
        </div>
        <div class="body">
            <h1><g:message code="default.list.label" args="[entityName]" /></h1>
            <g:if test="${flash.message}">
            <div class="message">${flash.message}</div>
            </g:if>
            <div class="list">
                <table>
                    <thead>
                        <tr>
                        
                            <g:sortableColumn property="id" title="${message(code: 'address.id.label', default: 'Id')}" />
                        
                            <g:sortableColumn property="firstName" title="${message(code: 'address.firstName.label', default: 'First Name')}" />
                        
                            <g:sortableColumn property="lastName" title="${message(code: 'address.lastName.label', default: 'Last Name')}" />
                        
                            <g:sortableColumn property="company" title="${message(code: 'address.company.label', default: 'Company')}" />
                        
                            <g:sortableColumn property="street" title="${message(code: 'address.street.label', default: 'Street')}" />
                        
                            <g:sortableColumn property="city" title="${message(code: 'address.city.label', default: 'City')}" />
                        
                        </tr>
                    </thead>
                    <tbody>
                    <g:each in="${addressInstanceList}" status="i" var="addressInstance">
                        <tr class="${(i % 2) == 0 ? 'odd' : 'even'}">
                        
                            <td><g:link action="show" id="${addressInstance.id}">${fieldValue(bean: addressInstance, field: "id")}</g:link></td>
                        
                            <td>${fieldValue(bean: addressInstance, field: "firstName")}</td>
                        
                            <td>${fieldValue(bean: addressInstance, field: "lastName")}</td>
                        
                            <td>${fieldValue(bean: addressInstance, field: "company")}</td>
                        
                            <td>${fieldValue(bean: addressInstance, field: "street")}</td>
                        
                            <td>${fieldValue(bean: addressInstance, field: "city")}</td>
                        
                        </tr>
                    </g:each>
                    </tbody>
                </table>
            </div>
            <div class="paginateButtons">
                <g:paginate total="${addressInstanceTotal}" />
            </div>
        </div>
    </body>
</html>
