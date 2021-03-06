<h3>Quick Links:</h3>
<ul>
  <li><?php print l('Configure settings', 'admin/tripal/tripal_bulk_loader_template/configure') ?></li>
  <li><?php print l('List Bulk Loader Jobs', 'admin/tripal/tripal_bulk_loader_template/jobs') ?></li>
  <li><?php print l('Manage Templates', 'admin/tripal/tripal_bulk_loader_template/manage_templates')?>
    <ul>
      <li><?php print l('Create a new template', 'admin/tripal/tripal_bulk_loader_template/manage_templates/create')?></li>
      <li><?php print l('Edit an existing template', 'admin/tripal/tripal_bulk_loader_template/manage_templates/edit')?></li>
      <li><?php print l('Delete an existing template', 'admin/tripal/tripal_bulk_loader_template/manage_templates/delete')?></li>        
      <li><?php print l('Import a new template', 'admin/tripal/tripal_bulk_loader_template/manage_templates/import')?></li>
      <li><?php print l('Export an existing template', 'admin/tripal/tripal_bulk_loader_template/manage_templates/export')?></li>
    </ul>
  </li>
</ul>
<br>
<h3>Module Description:</h3>
<p>This module allows you to create loading templates for any tab-delimited 
   data file which can be used repetitively for loading data into chado. The Loading Templates are a direct mapping 
   between the columns in your file and the columns in chado tables. To use this tool 
   you need to be very familar with the chado schema -See 
   <?php  l(t('Chado -Getting Started'), 'http://gmod.org/wiki/Chado_-_Getting_Started') ?>.
   The ability to add constants and specify values via foreign key contraints is also provided 
   in order for the loader to fill chado columns which may be required but are not specified 
   in your input file.
</p>
<br />